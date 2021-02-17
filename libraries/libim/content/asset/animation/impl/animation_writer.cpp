#include "../animation.h"
#include <libim/content/text/impl/text_resource_literals.h>

#include <utility>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::text;

constexpr static std::size_t vecIndent = 4;

void writeHeader(TextResourceWriter& rw, const Animation& anim, const std::vector<std::string>& headerComments)
{
    if(!headerComments.empty())
    {
        for (const auto& comment : headerComments) {
            rw.writeCommentLine(comment);
        }
        rw.writeEol();
    }

    rw.writeSection(kResName_Header)
      .writeEol()
      .writeKeyValue(kResName_Flags,  anim.flags(), 2)
      .writeKeyValue(kResName_Type,   anim.type(), 3)
      .writeKeyValue(kResName_Frames, anim.frames())
      .writeKeyValue(kResName_Fps,    utils::to_string<10,3>(anim.fps()), 4)
      .writeKeyValue(kResName_Joints, anim.joints())
      .writeEol()
      .writeEol();
}

void writeMarkers(TextResourceWriter& rw, const Animation& anim)
{
    if(anim.markers().empty()) {
        return;
    }

    rw.writeSection(kResName_Markers)
      .writeEol()
      .writeList(kResName_Markers, anim.markers(),
      [](TextResourceWriter& rw, auto /*idx*/, const auto& marker){
           rw.writeNumber<10,6>(marker.frame)
             .indent(1)
             .writeEnum(marker.type);
       })
      .writeEol()
      .writeEol();
}

void writeKeyframes(TextResourceWriter& rw, const Animation& anim)
{
    rw.writeSection(kResName_KfNodes)
      .writeEol()
      .writeList(kResName_Nodes, anim.nodes(),
      [](TextResourceWriter& rw, auto /*idx*/, const KeyNode& node)
      {

          rw.writeKeyValue(kResName_Node, node.num, 4);
          rw.writeKeyValue(kResName_MeshName, node.meshName);

          // Write frames
          const auto nEntries = node.entries.size();
          rw.writeList(kResName_Entries, node.entries,
          [nEntries](TextResourceWriter& rw, auto idx, const KeyNodeEntry& frame)
          {
              if(idx == 0)
              {
                  // Write list header info
                  rw.writeCommentLine("num:   frame:   flags:           x:           y:           z:           p:           y:           r:"sv);
                  rw.writeCommentLine("                                dx:          dy:          dz:          dp:          dy:          dr:"sv);
              }

              const auto rowBegin = rw.tell();

              rw.writeRowIdx(idx, utils::numdigits(nEntries) + 2);
              rw.indent(rw.getNumberIndent(9, frame.frame));
              rw.writeNumber(frame.frame);

              rw.indent(3);
              rw.writeFlags(frame.flags);

              const auto placementIndent = rw.tell() - rowBegin;

              rw.writeVector(frame.pos, vecIndent);
              rw.writeVector(frame.rot, vecIndent);

              rw.writeEol().indent(placementIndent);

              rw.writeVector(frame.dpos, vecIndent);
              rw.writeVector(frame.drot, vecIndent);
          });
      });
}

void libim::content::asset::keyWrite(const Animation& anim, text::TextResourceWriter& rw, const std::vector<std::string>& headerComments)
{
    writeHeader(rw, anim, headerComments);
    writeMarkers(rw, anim);
    writeKeyframes(rw, anim);
}

void libim::content::asset::keyWrite(const Animation& anim, text::TextResourceWriter&& rw, const std::vector<std::string>& headerComments)
{
    keyWrite(anim, rw, headerComments);
}
