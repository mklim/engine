#include <memory>
#include <vector>
#include "flutter/lib/ui/painting/canvas.h"
#include "flutter/lib/ui/painting/paint.h"
#include "flutter/lib/ui/painting/picture_recorder.h"
#include "flutter/testing/testing.h"
#include "flutter/testing/thread_test.h"

namespace flutter {
namespace testing {

using PictureRecorderTest = ThreadTest;

TEST_F(PictureRecorderTest, DetectsBigPictureObstruction) {
  auto recorder = PictureRecorder::Create();
  fml::RefPtr<Canvas> canvas =
      Canvas::Create(recorder.get(), /*left=*/0., /*top=*/0.,
                     /*right=*/500., /*bottom=*/500.);
  Paint paint = Paint();
  paint.paint_ = SkPaint();
  paint.paint_.setColor(SkColors::kCyan);
  paint.paint_.setStyle(SkPaint::Style::kFill_Style);
  paint.paint_.setStrokeWidth(20);
  paint.is_null_ = false;
  const auto paint_data = PaintData();

  // Draw a canvas with an opaque header and footer that's transparent inbetween
  // the two.
  // +----------+
  // |XXXXXXXXXX|
  // +----------+
  // |          |
  // |          |
  // +----------+
  // |XXXXXXXXXX|
  // +----------+
  SkRect footer =
      SkRect::MakeXYWH(/*x=*/0., /*y=*/0., /*width=*/500., /*height=*/100.);
  canvas->drawRect(footer.fLeft, footer.fTop, footer.fRight, footer.fBottom,
                   paint, paint_data);
  // We're deliberately drawing the header too wide. We want to test clipping
  // later on.
  SkRect header =
      SkRect::MakeXYWH(/*x=*/0., /*y=*/400., /*width=*/1000., /*height=*/100.);
  canvas->drawRect(header.fLeft, header.fTop, header.fRight, header.fBottom,
                   paint, paint_data);
  canvas->save();
  sk_sp<SkPicture> picture = recorder->endRecordingDebug();

  auto hits = std::vector<std::unique_ptr<SkRect>>();
  // Empty queries have no results.
  SkRect unobstructed =
      SkRect::MakeXYWH(/*x=*/0., /*y=*/200., /*width=*/500., /*height=*/100.);
  picture->search(unobstructed, &hits);
  ASSERT_TRUE(hits.empty());
  hits.clear();

  // Hits that partially overlap with a drawn area return bboxes describing the
  // intersection of the query and the drawn area.
  SkRect one_hit =
      SkRect::MakeXYWH(/*x=*/0., /*y=*/0., /*width=*/250., /*height=*/250.);
  picture->search(one_hit, &hits);
  ASSERT_EQ(1UL, hits.size());
  ASSERT_EQ(*hits[0], SkRect::MakeXYWH(/*x=*/0., /*y=*/0., /*width=*/250.,
                                       /*height=*/100.));
  hits.clear();

  // Hits that overlap with multiple drawn areas return bboxes describing the
  // intersections of the query and the drawn area. Hits are clipped to the
  // canvas size.
  SkRect total = SkRect::MakeLTRB(/*left=*/0., /*top=*/0., /*right=*/1000.,
                                  /*bottom=*/1000.);
  picture->search(total, &hits);
  ASSERT_EQ(2UL, hits.size());
  ASSERT_EQ(*hits[0], footer);
  SkRect clipped_expected_hit = SkRect(header);
  clipped_expected_hit.fRight = 500.;
  ASSERT_EQ(*hits[1], clipped_expected_hit);
}

TEST_F(PictureRecorderTest, DetectsSimplePictureObstruction) {
  auto recorder = PictureRecorder::Create();
  fml::RefPtr<Canvas> canvas =
      Canvas::Create(recorder.get(), /*left=*/0., /*top=*/0.,
                     /*right=*/500., /*bottom=*/500.);
  Paint paint = Paint();
  paint.paint_ = SkPaint();
  paint.paint_.setColor(SkColors::kCyan);
  paint.paint_.setStyle(SkPaint::Style::kFill_Style);
  paint.paint_.setStrokeWidth(20);
  paint.is_null_ = false;
  const auto paint_data = PaintData();

  // Draw a canvas with a single piece of UI.
  // +----------+
  // |          |
  // |          |
  // |          |
  // |          |
  // +----------+
  // |XXXXXXXXXX|
  // +----------+
  SkRect footer =
      SkRect::MakeXYWH(/*x=*/0., /*y=*/0., /*width=*/500., /*height=*/100.);
  canvas->drawRect(footer.fLeft, footer.fTop, footer.fRight, footer.fBottom,
                   paint, paint_data);
  canvas->save();
  sk_sp<SkPicture> picture = recorder->endRecordingDebug();

  auto hits = std::vector<std::unique_ptr<SkRect>>();
  // Empty queries have no results.
  SkRect unobstructed =
      SkRect::MakeXYWH(/*x=*/0., /*y=*/200., /*width=*/500., /*height=*/100.);
  picture->search(unobstructed, &hits);
  ASSERT_TRUE(hits.empty());
  hits.clear();

  // Hits that partially overlap with a drawn area return bboxes describing the
  // intersection of the query and the drawn area.
  SkRect one_hit =
      SkRect::MakeXYWH(/*x=*/0., /*y=*/0., /*width=*/1000., /*height=*/250.);
  picture->search(one_hit, &hits);
  ASSERT_EQ(1UL, hits.size());
  ASSERT_EQ(*hits[0], SkRect::MakeXYWH(/*x=*/0., /*y=*/0., /*width=*/500.,
                                       /*height=*/100.));
  hits.clear();
}

TEST_F(PictureRecorderTest, DetectsOpaqueBackground) {
  auto recorder = PictureRecorder::Create();
  fml::RefPtr<Canvas> canvas =
      Canvas::Create(recorder.get(), /*left=*/0., /*top=*/0.,
                     /*right=*/500., /*bottom=*/500.);

  // Draw a completely opaque rect.
  SkColor cyan = SkColors::kCyan.toSkColor();
  canvas->drawColor(cyan, SkBlendMode::kSrcOver);
  sk_sp<SkPicture> picture = recorder->endRecordingDebug();

  // Queries should return their size clipped to the canvas area.
  auto hits = std::vector<std::unique_ptr<SkRect>>();
  picture->search(
      SkRect::MakeXYWH(/*x=*/0., /*y=*/200., /*width=*/1000., /*height=*/100.),
      &hits);
  ASSERT_EQ(1UL, hits.size());
  ASSERT_EQ(*hits[0], SkRect::MakeXYWH(/*x=*/0., /*y=*/200., /*width=*/500.,
                                       /*height=*/100.));
}

}  // namespace testing
}  // namespace flutter
