#include "../animation.h"
#include "../../../text/impl/text_resource_literals.h"

#include <utility>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::text;

constexpr static std::size_t vecIndent = 4;

void WriteHeader(TextResourceWriter& rw, const Animation& anim, std::string_view headerComment)
{
    if(!headerComment.empty())
    {
        rw.writeCommentLine(headerComment)
          .writeEol();
    }

    rw.writeSection(kResName_Header)
      .writeEol()
      .writeKeyValue<decltype(anim.flages()), 2>(kResName_Flags, anim.flages())
      .writeKeyValue<decltype(anim.type()), 3>(kResName_Type, anim.type())
      .writeKeyValue<uint32_t>(kResName_Frames, anim.frames())
      .writeKeyValue<std::string, 4>(kResName_Fps, utils::to_string<10,3>(anim.fps()))
      .writeKeyValue<uint32_t>(kResName_Joints, anim.joints())
      .writeEol()
      .writeEol();
}

void WriteMarkers(TextResourceWriter& rw, const Animation& anim)
{
    if(anim.markers().empty()) {
        return;
    }

    rw.writeSection(kResName_Markers)
      .writeEol()
      .writeList(kResName_Markers, anim.markers(),
      [](TextResourceWriter& rw, [[maybe_unused]]auto idx, const auto& marker){
           rw.writeNumber<10,6>(marker.frame)
             .indent(1)
             .writeEnum(marker.type);
       })
      .writeEol()
      .writeEol();
}

void WriteKeyframes(TextResourceWriter& rw, const Animation& anim)
{
    rw.writeSection(kResName_KfNodes)
      .writeEol()
      .writeList(kResName_Nodes, anim.nodes(),
      [](TextResourceWriter& rw, [[maybe_unused]]auto idx, const AnimationNode& node)
      {

          rw.writeKeyValue<decltype(node.id), 4>(kResName_Node, node.id);
          rw.writeKeyValue<std::string_view>(kResName_MeshName, node.meshName);

          // Write frames
          const auto nFrames = node.frames.size();
          rw.writeList(kResName_Entries, node.frames,
          [nFrames](TextResourceWriter& rw, auto idx, const Keyframe& frame)
          {
              if(idx == 0)
              {
                  // Write list header info
                  rw.writeCommentLine("num:   frame:   flags:           x:           y:           z:           p:           y:           r:"sv);
                  rw.writeCommentLine("                                dx:          dy:          dz:          dp:          dy:          dr:"sv);
              }

              const auto rowBegin = rw.tell();

              rw.writeRowIdx(idx, utils::numdigits(nFrames));
              rw.indent(rw.getNumberIndent(9, frame.number));
              rw.writeNumber(frame.number);

              rw.indent(3);
              rw.writeFlags(frame.flags);

              const auto placementIndent = rw.tell() - rowBegin;

              rw.writeVector<vecIndent>(frame.pos);
              rw.writeVector<vecIndent>(frame.rot);

              rw.writeEol().indent(placementIndent);

              rw.writeVector<vecIndent>(frame.dpos);
              rw.writeVector<vecIndent>(frame.drot);
          });
      });
}

void Animation::serialize(TextResourceWriter& rw, std::string_view headerComment) const
{
    WriteHeader(rw, *this, headerComment);
    WriteMarkers(rw, *this);
    WriteKeyframes(rw, *this);
}

void Animation::serialize(TextResourceWriter&& rw, std::string_view headerComment) const
{
    serialize(rw, headerComment);
}
