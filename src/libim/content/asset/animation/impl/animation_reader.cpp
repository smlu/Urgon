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
    rr.assertSection(kResName_Markers);
    auto markers = rr.readList<KeyMarker, false>(kResName_Markers,
    [](TextResourceReader& rr, auto& m){
        m.frame = rr.getNumber<decltype(m.frame)>();
        m.type  = rr.readFlags<decltype(m.type)>();
    });

    anim.setMarkers(std::move(markers));
}

void ParseKeyframes(TextResourceReader& rr, Animation& anim)
{
    auto nodes = rr.readList<AnimationNode, false>(kResName_Nodes,
    [&](TextResourceReader& rr, auto& node)
    {
        node.id = rr.readKey<decltype(node.id)>(kResName_Node);
        node.meshName = rr.readKey<std::string>(kResName_MeshName);

        node.frames = rr.readList<Keyframe>(kResName_Entries,
        [](TextResourceReader& rr, auto& frame)
        {
            frame.number = rr.getNumber<decltype(frame.number)>();
            frame.flags  = rr.readFlags<decltype(frame.flags)>();

            frame.pos = rr.getVector<FVector3>();
            frame.rot = rr.getVector<FRotator>();

            frame.dpos = rr.getVector<FVector3>();
            frame.drot = rr.getVector<FRotator>();
        });
    });

    anim.setNodes(std::move(nodes));
}


Animation Animation::load(TextResourceReader& rr)
{
    Animation anim;

    rr.assertSection(kResName_Header);
    ParseHeader(rr, anim);

    std::string section = rr.readSection();
    if(section == kResName_Markers)
    {
        ParseMarkers(rr, anim);
        rr.assertSection(kResName_KfNodes);
    } else if(section != kResName_KfNodes) {
        //TODO: throw
    }

    ParseKeyframes(rr, anim);

    return anim;
}
