#include "../animation.h"
#include "../../../text/impl/text_resource_literals.h"

#include <utility>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::text;


void ParseHeader(TextResourceReader& rr, Animation& anim)
{
    auto flags  = rr.readKey<Animation::Flag>(kResName_Flags);
    auto type   = rr.readKey<Animation::Type>(kResName_Type);
    auto frames = rr.readKey<uint32_t>(kResName_Frames);
    auto fps    = rr.readKey<float>(kResName_Fps);
    auto joints = rr.readKey<uint32_t>(kResName_Joints);

    anim.setFlags(flags);
    anim.setType(type);
    anim.setFrames(frames);
    anim.setFps(fps);
    anim.setJoints(joints);
}

void ParseMarkers(TextResourceReader& rr, Animation& anim)
{
    auto markers = rr.readList<KeyMarker, false>(kResName_Markers,
    [](TextResourceReader& rr, auto& m){
        m.frame = rr.getNumber<decltype(m.frame)>();
        m.type  = rr.readFlags<decltype(m.type)>();
    });

    anim.setMarkers(std::move(markers));
}

void ParseKeyframes(TextResourceReader& rr, Animation& anim)
{
    auto nodes = rr.readList<KeyNode, false>(kResName_Nodes,
    [&](TextResourceReader& rr, auto& node)
    {
        node.num = rr.readKey<decltype(node.num)>(kResName_Node);
        node.meshName = rr.readKey<std::string>(kResName_MeshName);

        node.entries = rr.readList<KeyNodeEntry>(kResName_Entries,
        [](TextResourceReader& rr, auto& entry)
        {
            entry.frame = rr.getNumber<decltype(entry.frame)>();
            entry.flags  = rr.readFlags<decltype(entry.flags)>();

            entry.pos = rr.getVector<Vector3f>();
            entry.rot = rr.getVector<FRotator>();

            entry.dpos = rr.getVector<Vector3f>();
            entry.drot = rr.getVector<FRotator>();
        });
    });

    anim.setNodes(std::move(nodes));
}


Animation& Animation::deserialize(TextResourceReader& rr)
{
    rr.assertSection(kResName_Header);
    ParseHeader(rr, *this);

    Token t;
    rr.readSection(t);
    if(t.value() == kResName_Markers)
    {
        ParseMarkers(rr, *this);
        rr.assertSection(kResName_KfNodes);
    }
    else if(t.value() != kResName_KfNodes)
    {
        LOG_DEBUG("Animation::load: section expected '%', found '%'", kResName_KfNodes, t.value());
        throw TokenizerError("expected section: KEYFRAME NODES"sv, t.location());
    }

    ParseKeyframes(rr, *this);
    this->setName(GetFilename(rr.istream().name()));
    return *this;
}

Animation& Animation::deserialize(TextResourceReader&& rr)
{
    return deserialize(rr);
}
