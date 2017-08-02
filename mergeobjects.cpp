#include "mergeobjects.h"
#include "widget.h"

#include <ctype.h>
#include <iostream>
#include <utility>

#include <AnnotatorLib/Annotation.h>
#include <AnnotatorLib/Commands/MergeObjects.h>
#include <AnnotatorLib/Frame.h>
#include <AnnotatorLib/Session.h>
#include <dlib/opencv.h>
#include <QDebug>
#include <QtGui/QPainter>
#include <opencv2/core/core.hpp>
#include <opencv2/video/tracking.hpp>

using namespace Annotator::Plugins;

MergeObjects::MergeObjects() {}

MergeObjects::~MergeObjects() {}

QString MergeObjects::getName() { return "MergeObjects"; }

QWidget *MergeObjects::getWidget() { return &widget; }

bool MergeObjects::setFrame(shared_ptr<Frame> frame, cv::Mat image) {
  this->lastFrame = this->frame;
  this->lastFrameImg = this->frameImg;
  this->frame = frame;
  this->frameImg = image;
  return lastFrame != frame;
}

void MergeObjects::setObject(shared_ptr<Object> object) {}

shared_ptr<Object> MergeObjects::getObject() const {
  return shared_ptr<Object>();
}

void MergeObjects::setLastAnnotation(shared_ptr<Annotation> annotation) {}

std::vector<shared_ptr<Commands::Command>> MergeObjects::getCommands() {
  std::vector<shared_ptr<Commands::Command>> commands;
  if (frame == nullptr || lastFrame == nullptr ||
      lastFrame->getId() != frame->getId() - 1)
    return commands;

  try {
    dlib::cv_image<dlib::bgr_pixel> cvimg(this->lastFrameImg);
    for (auto &pair : lastFrame->getAnnotations()) {
      auto annotation = pair.second.lock();
      if (annotation->isLast()) {
        tracker.start_track(
            cvimg,
            dlib::rectangle(annotation->getX(), annotation->getY(),
                            annotation->getX() + annotation->getWidth(),
                            annotation->getY() + annotation->getHeight()));
        std::pair<cv::Rect, float> found = findObject();
        for (auto &pair2 : frame->getAnnotations()) {
          auto a2 = pair2.second.lock();
          if (a2->isFirst()) {
            cv::Rect rect(a2->getX(), a2->getY(), a2->getWidth(),
                          a2->getHeight());
            float IoU = intersectionOverUnion(found.first, rect);
            if (IoU > threshold) {
              shared_ptr<Commands::MergeObjects> mO =
                  std::make_shared<Commands::MergeObjects>(
                      project->getSession(), annotation->getObject(),
                      a2->getObject());
              commands.push_back(mO);
            }
          }
        }
      }
    }
  } catch (std::exception &e) {
    qDebug() << e.what();
  }

  return commands;
}

void MergeObjects::setThreshold(float threshold) {
  this->threshold = threshold;
}

// source:
// https://putuyuwono.wordpress.com/2015/06/26/intersection-and-union-two-rectangles-opencv/
float MergeObjects::intersectionOverUnion(cv::Rect &r1, cv::Rect &r2) {
  cv::Rect rectIntersection = r1 & r2;
  cv::Rect rectUnion = r1 | r2;
  return 1.0f * rectIntersection.area() / rectUnion.area();
}

std::pair<cv::Rect, float> MergeObjects::findObject() {
  auto ret = std::make_pair(cv::Rect(), 0.0f);
  dlib::cv_image<dlib::bgr_pixel> cvimg(this->frameImg);
  try {
    ret.second = tracker.update(cvimg);  // returns the peak to side-lobe
                                         // ratio.  This is a number that
                                         // measures how
    dlib::rectangle found = tracker.get_position();
    ret.first =
        cv::Rect(found.left(), found.top(), found.width(), found.height());
  } catch (std::exception &e) {
  }
  return ret;
}
