
#include "CameraViewportReporter.h"
#include "Camera/ICamera.h"
#include "AIKnowledge.h"

using namespace game;

CameraViewportReporter::CameraViewportReporter(const mono::ICameraPtr& camera)
    : m_camera(camera)
{ }

void CameraViewportReporter::doUpdate(unsigned int delta)
{
    const math::Quad& viewport = m_camera->GetViewport();
    g_camera_viewport = math::Quad(viewport.mA, viewport.mA + viewport.mB);
}
