#ifndef ANNOTATORPLUGIN_MERGEOBJECTS_H
#define ANNOTATORPLUGIN_MERGEOBJECTS_H

#include "widget.h"

#include <annotator/plugins/plugin.h>
#include <dlib/image_processing.h>
#include <QtCore/QObject>
#include <QtCore/QtPlugin>
#include <QtGui/QIcon>
#include <opencv2/core/mat.hpp>

using namespace AnnotatorLib;
using std::shared_ptr;

namespace AnnotatorLib {
class Session;
}

namespace Annotator {
namespace Plugins {

class MergeObjects : public Plugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "annotator.mergeobjects" FILE "mergeobjects.json")
  Q_INTERFACES(Annotator::Plugin)

 public:
  MergeObjects();
  ~MergeObjects();
  QString getName() override;
  QWidget *getWidget() override;

  bool setFrame(shared_ptr<Frame> frame, cv::Mat image) override;
  void setObject(shared_ptr<Object> object) override;
  shared_ptr<Object> getObject() const override;
  void setLastAnnotation(shared_ptr<Annotation> annotation) override;
  std::vector<shared_ptr<Commands::Command>> getCommands() override;
  virtual bool requiresObject() const override { return false; }
  void setThreshold(float threshold);

 protected:
  cv::Mat frameImg;
  cv::Mat lastFrameImg;

  Widget widget;

  shared_ptr<Frame> frame = 0;
  shared_ptr<Frame> lastFrame = 0;

  dlib::correlation_tracker tracker;

  float threshold = 0.5f;

  std::pair<cv::Rect, float> findObject();
  float intersectionOverUnion(cv::Rect &r1, cv::Rect &r2);
};
}
}

#endif  // ANNOTATORPLUGIN_MERGEOBJECTS_H
