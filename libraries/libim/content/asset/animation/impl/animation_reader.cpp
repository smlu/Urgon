#include "../animation.h"
#include <libim/content/text/impl/text_resource_literals.h>

#include <string_view>
#include <utility>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::text;
using namespace std::string_view_literals;


void parseHeader(TextResourceReader& rr, Animation& anim)
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

void parseMarkers(TextResourceReader& rr, Animation& anim)
{
    auto markers = rr.readList<std::vector<KeyMarker>, false>(kResName_Markers,
    [](TextResourceReader& rr, auto /*rowIdx*/, auto& m){
        m.frame = rr.getNumber<decltype(m.frame)>();
        m.type  = rr.readFlags<decltype(m.type)>();
    });

    anim.setMarkers(std::move(markers));
}

void parseKeyframes(TextResourceReader& rr, Animation& anim)
{
    auto nodes = rr.readList<std::vector<KeyNode>, false>(kResName_Nodes,
    [&](TextResourceReader& rr, auto /*rowIdx*/, auto& node)
    {
        node.num      = rr.readKey<decltype(node.num)>(kResName_Node);
        node.meshName = rr.readKey<std::string>(kResName_MeshName);

        node.entries = rr.readList<std::vector<KeyNodeEntry>>(kResName_Entries,
        [](TextResourceReader& rr, auto /*rowIdx*/, auto& entry)
        {
            entry.frame = rr.getNumber<decltype(entry.frame)>();
            entry.flags = rr.readFlags<decltype(entry.flags)>();

            entry.pos   = rr.readVector<Vector3f>();
            entry.rot   = rr.readVector<FRotator>();

            entry.dpos  = rr.readVector<Vector3f>();
            entry.drot  = rr.readVector<FRotator>();
        });
    });

    anim.setNodes(std::move(nodes));
}

Animation libim::content::asset::keyLoad(text::TextResourceReader& rr)
{
    Animation anim;
    rr.assertSection(kResName_Header);
    parseHeader(rr, anim);

    auto section = rr.readSection();
    if(section == kResName_Markers)
    {
        parseMarkers(rr, anim);
        rr.assertSection(kResName_KfNodes);
    }
    else if(section != kResName_KfNodes)
    {
        LOG_DEBUG("Animation::load: section expected '%', found '%'", kResName_KfNodes, section);
        throw TokenizerError("expected section: KEYFRAME NODES"sv, rr.currentToken().location());
    }

    parseKeyframes(rr, anim);
    anim.setName(getFilename(rr.istream().name()));
    return anim;
}

Animation libim::content::asset::keyLoad(text::TextResourceReader&& rr)
{
    return keyLoad(rr);
}
