#include "Renderer/pch.hpp"
#include "View.hpp"

#include "Renderer/RendererCommon.hpp"

using namespace D_MATH;

namespace Darius::Renderer
{
	void IView::FillPlanarViewConstants(GlobalConstants& constants) const
	{
		Matrix4 tempMat;

		// View
		constants.View = GetViewMatrix();
		constants.InvView = GetInverseViewMatrix();

		// Proj
		constants.Proj = GetProjectionMatrix();
		constants.InvProj = GetInverseProjectionMatrix();

		// View Proj
		tempMat = GetProjectionMatrix();
		constants.ViewProj = tempMat;
		constants.InvViewProj = GetInverseViewProjectionMatrix();

		auto viewProjEyeCenter = tempMat * Matrix4::MakeLookToward(Vector3::Zero, GetViewDirection(), GetViewUpDirection());
		constants.InvViewProjEyeCenter = Matrix4::Transpose(Matrix4::Inverse(viewProjEyeCenter));

		auto frustum = GetWorldFrustum();

		for (int i = 0; i < 6; i++)
		{
			auto plane = frustum.GetFrustumPlane((D_MATH_CAMERA::Frustum::PlaneID)i).GetVector4();
			constants.FrustumPlanes[i] = plane;
		}

		constants.CameraPos = GetViewOrigin();

		auto rect = GetViewExtent();
		auto sizeV2 = Vector2((float)rect.GetWidth(), (float)rect.GetHeight());
		constants.RenderTargetSize = sizeV2;
		constants.InvRenderTargetSize = D_MATH::Recip(sizeV2);
		constants.NearZ = GetNearZ();
		constants.FarZ = GetFarZ();
	}

	uint32_t IView::GetNumChildViews(ViewType::Enum supportedTypes) const
	{
		(void)supportedTypes;
		return 1u;
	}

	IView const* IView::GetChildView(ViewType::Enum supportedTypes, uint32_t index) const
	{
		(void)supportedTypes;
		D_ASSERT(index == 0);
		return this;
	}


	void PlanarView::SetViewport(D_GRAPHICS::Viewport const& viewport)
	{
		mViewport = viewport;
		mScissorRect = viewport.Rect();
		mCacheValid = false;
	}

	void PlanarView::SetPixelOffset(D_MATH::Vector2 const& offset)
	{
		mPixelOffset = offset;
		mCacheValid = false;
	}

	void PlanarView::SetCamera(D_MATH_CAMERA::Camera const& cam)
	{
		mCameraMath = cam;
		mCacheValid = false;
	}

	void PlanarView::UpdateCache()
	{
		if (mCacheValid)
			return;

		mPixelOffsetMatrix = Matrix4(AffineTransform(Vector3(
			2.f * mPixelOffset.GetX() / (mViewport.MaxX - mViewport.MinX),
			-2.f * mPixelOffset.GetY() / (mViewport.MaxY - mViewport.MinY),
			0.f
		)));
		mInvPixelOffsetMatrix = Matrix4::Inverse(mPixelOffsetMatrix);

		mCacheValid = true;
	}

	D_GRAPHICS::ViewportState PlanarView::GetViewportState() const
	{
		return D_GRAPHICS::ViewportState()
			.AddViewport(mViewport)
			.AddScissorRect(mScissorRect);
	}

	void PlanarView::EnsureCacheIsValid() const
	{
		D_ASSERT(mCacheValid);
	}

	bool PlanarView::IsReverseDepth() const
	{
		return mCameraMath.IsReverseZ();
	}

	bool PlanarView::IsOrthographicProjection() const
	{
		mCameraMath.IsOrthographic();
	}

	bool PlanarView::IsStereoView() const
	{
		return false;
	}

	bool PlanarView::IsCubemapView() const
	{
		return false;
	}

	bool PlanarView::IsBoxVisible(Darius::Math::Bounds::Aabb const& aabb) const
	{
		EnsureCacheIsValid();

		return mCameraMath.GetWorldSpaceFrustum().Intersects(aabb);
	}

	bool PlanarView::IsMirrored() const
	{
		EnsureCacheIsValid();
		return mCameraMath.IsMirrored();
	}

	D_MATH::Vector3 PlanarView::GetViewOrigin() const
	{
		EnsureCacheIsValid();
		return mCameraMath.GetPosition();
	}

	D_MATH::Vector3 PlanarView::GetViewDirection() const
	{
		EnsureCacheIsValid();
		return mCameraMath.GetForwardVec();
	}

	D_MATH::Vector3 PlanarView::GetViewUpDirection() const
	{
		EnsureCacheIsValid();
		return mCameraMath.GetUpVec();
	}

	D_MATH_CAMERA::Frustum PlanarView::GetViewFrustum() const
	{
		EnsureCacheIsValid();
		return mCameraMath.GetViewSpaceFrustum();
	}

	D_MATH_CAMERA::Frustum PlanarView::GetWorldFrustum() const
	{
		EnsureCacheIsValid();
		return mCameraMath.GetWorldSpaceFrustum();
	}

	D_MATH::Matrix4 PlanarView::GetViewMatrix() const
	{
		EnsureCacheIsValid();
		return mCameraMath.GetViewMatrix();
	}

	D_MATH::Matrix4 PlanarView::GetInverseViewMatrix() const
	{
		EnsureCacheIsValid();
		return Matrix4::Inverse(mCameraMath.GetViewMatrix());
	}

	D_MATH::Matrix4 PlanarView::GetProjectionMatrix(bool includeOffset) const
	{
		(void)includeOffset;
		EnsureCacheIsValid();
		return mCameraMath.GetProjMatrix();
	}

	D_MATH::Matrix4 PlanarView::GetInverseProjectionMatrix(bool includeOffset) const
	{
		(void)includeOffset;
		EnsureCacheIsValid();
		return Matrix4::Inverse(mCameraMath.GetProjMatrix());
	}

	D_MATH::Matrix4 PlanarView::GetViewProjectionMatrix(bool includeOffset) const
	{
		(void)includeOffset;
		EnsureCacheIsValid();
		return mCameraMath.GetViewProjMatrix();
	}

	D_MATH::Matrix4 PlanarView::GetInverseViewProjectionMatrix(bool includeOffset) const
	{
		(void)includeOffset;
		EnsureCacheIsValid();
		return Matrix4::Inverse(mCameraMath.GetViewProjMatrix());
	}

	D_MATH::Rect PlanarView::GetViewExtent() const
	{
		return mScissorRect;
	}

	D_MATH::Vector2 PlanarView::GetPixelOffset() const
	{
		return mPixelOffset;
	}

	float PlanarView::GetNearZ() const
	{
		EnsureCacheIsValid();
		return mCameraMath.GetNearClip();
	}

	float PlanarView::GetFarZ() const
	{
		EnsureCacheIsValid();
		return mCameraMath.GetFarClip();
	}


	void CompositeView::AddView(std::shared_ptr<IView> view)
	{
		mChildViews.push_back(view);
	}

	uint32_t CompositeView::GetNumChildViews(ViewType::Enum supportedTypes) const
	{
		(void)supportedTypes;
		return uint32_t(mChildViews.size());
	}

	IView const* CompositeView::GetChildView(ViewType::Enum supportedTypes, uint32_t index) const
	{
		D_ASSERT((uint32_t)mChildViews.size() > index);

		(void)supportedTypes;
		return mChildViews[index].get();;
	}

}