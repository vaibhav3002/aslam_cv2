#include <aslam/frames/visual-nframe.h>
#include <aslam/common/predicates.h>
namespace aslam {

VisualNFrame::VisualNFrame() { }

VisualNFrame::VisualNFrame(const aslam::NFramesId& id,
                           unsigned int num_frames)
: id_(id) {
  frames_.resize(num_frames);
}

VisualNFrame::VisualNFrame(const aslam::NFramesId& id,
                           std::shared_ptr<NCamera> ncameras)
: id_(id), cameraRig_(ncameras) {
  CHECK_NOTNULL(cameraRig_.get());
  frames_.resize(ncameras->getNumCameras());
}

VisualNFrame::VisualNFrame(std::shared_ptr<NCamera> ncameras)
: cameraRig_(ncameras) {
  CHECK_NOTNULL(cameraRig_.get());
  id_.randomize();
  frames_.resize(ncameras->getNumCameras());
}

VisualNFrame::~VisualNFrame() { }

/// \brief get the camera rig
const NCamera& VisualNFrame::getNCameras() const {
  CHECK_NOTNULL(cameraRig_.get());
  return *cameraRig_;
}

/// \brief get the camera rig
NCamera::Ptr VisualNFrame::getNCamerasMutable() {
  return cameraRig_;
}

/// \brief set the camera rig.
void VisualNFrame::setNCameras(NCamera::Ptr ncameras) {
  CHECK(ncameras != nullptr);
  cameraRig_ = ncameras;
  CHECK_EQ(frames_.size(), cameraRig_->numCameras()) << "Number of cameras "
      << "in camera system does not match the current number of frames.";

  // Also assign the camera to the existing non-NULL frames.
  for (unsigned int i = 0; i < frames_.size(); ++i) {
    if (frames_[i] != nullptr) {
      const bool is_already_assigned = frames_[i]->getCameraGeometry().get()
          == cameraRig_->getCameraShared(i).get();
      if (is_already_assigned) {
        // This camera was already assigned to the visual frame. Most probably
        // used did it manually before.
        continue;
      }
      CHECK(frames_[i]->getCameraGeometry() == nullptr) << "Visual frame "
          << "with index " << i << " has been already assigned a camera "
          << frames_[i]->getCameraGeometry()->getId()
          << " which is not matching the new camera "
          << cameraRig_->getCameraShared(i);
      frames_[i]->setCameraGeometry(cameraRig_->getCameraShared(i));
    }
    CHECK_EQ(frames_[i]->getCameraGeometry().get(),
             cameraRig_->getCameraShared(i).get());
  }
}

/// \brief get one frame
const VisualFrame& VisualNFrame::getFrame(size_t frameIndex) const {
  CHECK_LT(frameIndex, frames_.size());
  CHECK_NOTNULL(frames_[frameIndex].get());
  return *frames_[frameIndex];
}

/// \brief get one frame
VisualFrame::Ptr VisualNFrame::getFrameMutable(size_t frameIndex) {
  CHECK_LT(frameIndex, frames_.size());
  return frames_[frameIndex];  
}

/// \brief the number of frames
size_t VisualNFrame::getNumFrames() const {
  return frames_.size();
}

/// \brief the number of frames
size_t VisualNFrame::getNumCameras() const {
  CHECK_NOTNULL(cameraRig_.get());
  return cameraRig_->getNumCameras();
}

/// \brief get the pose of body frame with respect to the camera i
const Transformation& VisualNFrame::get_T_C_B(size_t cameraIndex) const {
  CHECK_NOTNULL(cameraRig_.get());
  return cameraRig_->get_T_C_B(cameraIndex);
}

/// \brief get the geometry object for camera i
const Camera& VisualNFrame::getCamera(size_t cameraIndex) const {
  CHECK_NOTNULL(cameraRig_.get());
  return cameraRig_->getCamera(cameraIndex);
}

/// \brief gets the id for the camera at index i
const CameraId& VisualNFrame::getCameraId(size_t cameraIndex) const {
  CHECK_NOTNULL(cameraRig_.get());
  return cameraRig_->getCameraId(cameraIndex);
}

/// \brief does this rig have a camera with this id
bool VisualNFrame::hasCameraWithId(const CameraId& id) const {
  CHECK_NOTNULL(cameraRig_.get());
  return cameraRig_->hasCameraWithId(id);
}

/// \brief get the index of the camera with the id
/// @returns -1 if the rig doesn't have a camera with this id
size_t VisualNFrame::getCameraIndex(const CameraId& id) const {
  CHECK_NOTNULL(cameraRig_.get());
  return cameraRig_->getCameraIndex(id);
}

void VisualNFrame::setFrame(size_t frameIndex, VisualFrame::Ptr frame) {
  CHECK_LT(frameIndex, frames_.size());
  if (cameraRig_ != nullptr) {
    CHECK_EQ(&cameraRig_->getCamera(frameIndex), frame->getCameraGeometry().get());
  }
  frames_[frameIndex] = frame;
}

bool VisualNFrame::isFrameNull(size_t frameIndex) const {
  CHECK_LT(frameIndex, frames_.size());
  return static_cast<bool>(frames_[frameIndex]);
}

/// \brief binary equality
bool VisualNFrame::operator==(const VisualNFrame& other) const {
  bool same = true;

  same &= id_ == other.id_;
  same &= aslam::checkSharedEqual(cameraRig_, other.cameraRig_);
  same &= frames_.size() == other.frames_.size();
  if(same) {
    for(size_t i = 0; i < frames_.size(); ++i) {
      same &= aslam::checkSharedEqual(frames_[i], other.frames_[i]);
    }
  }
  return same;
}


} // namespace aslam