#pragma once

#include <Graphics/Viewport.hpp>
#include <Math/Camera/Frustum.hpp>
#include <Utils/Common.hpp>

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Math::Bounds
{
    class Aabb;
}

namespace Darius::Renderer
{
	class IView;
    struct GlobalConstants;

	namespace ViewType
	{
		enum Enum
		{
			Planar = 0x01,
			Stereo = 0x02,
			Cubemap = 0x04
		};
	}

	class ICompositeView
	{
	public:

		NODISCARD virtual uint32_t GetNumChildViews(ViewType::Enum supportedTypes) const = 0;
		NODISCARD virtual IView const* GetChildView(ViewType::Enum supportedTypes, uint32_t index) const = 0;

		virtual ~ICompositeView() = default;
	};

	class IView : public ICompositeView
    {
    public:
        virtual void FillPlanarViewConstants(GlobalConstants& constants) const;

        NODISCARD virtual D_GRAPHICS::ViewportState GetViewportState() const = 0;
        //NODISCARD virtual VariableRateShadingState GetVariableRateShadingState() const = 0;
        //NODISCARD virtual TextureSubresourceSet GetSubresources() const = 0;
        NODISCARD virtual bool                      IsReverseDepth() const = 0;
        NODISCARD virtual bool                      IsOrthographicProjection() const = 0;
        NODISCARD virtual bool                      IsStereoView() const = 0;
        NODISCARD virtual bool                      IsCubemapView() const = 0;
        NODISCARD virtual bool                      IsBoxVisible(Darius::Math::Bounds::Aabb const& bbox) const = 0;
        NODISCARD virtual bool                      IsMirrored() const = 0;
        NODISCARD virtual D_MATH::Vector3           GetViewOrigin() const = 0;
        NODISCARD virtual D_MATH::Vector3           GetViewDirection() const = 0;
        NODISCARD virtual D_MATH::Vector3           GetViewUpDirection() const = 0;
        NODISCARD virtual D_MATH_CAMERA::Frustum    GetViewFrustum() const = 0;
        NODISCARD virtual D_MATH_CAMERA::Frustum    GetWorldFrustum() const = 0;
        NODISCARD virtual D_MATH::Matrix4           GetViewMatrix() const = 0;
        NODISCARD virtual D_MATH::Matrix4           GetInverseViewMatrix() const = 0;
        NODISCARD virtual D_MATH::Matrix4           GetProjectionMatrix(bool includeOffset = true) const = 0;
        NODISCARD virtual D_MATH::Matrix4           GetInverseProjectionMatrix(bool includeOffset = true) const = 0;
        NODISCARD virtual D_MATH::Matrix4           GetViewProjectionMatrix(bool includeOffset = true) const = 0;
        NODISCARD virtual D_MATH::Matrix4           GetInverseViewProjectionMatrix(bool includeOffset = true) const = 0;
        NODISCARD virtual D_MATH::Rect              GetViewExtent() const = 0;
        NODISCARD virtual D_MATH::Vector2           GetPixelOffset() const = 0;
        NODISCARD virtual float                     GetNearZ() const = 0;
        NODISCARD virtual float                     GetFarZ() const = 0;

        NODISCARD uint32_t                          GetNumChildViews(ViewType::Enum supportedTypes) const override;
        NODISCARD IView const*                      GetChildView(ViewType::Enum supportedTypes, uint32_t index) const override;
    };

    class PlanarView : public IView
    {
    public:
        void                                        SetViewport(D_GRAPHICS::Viewport const& viewport);
        //void SetVariableRateShadingState(VariableRateShadingState shadingRateState);
        void                                        SetPixelOffset(D_MATH::Vector2 const& offset);
        void                                        SetCamera(D_MATH_CAMERA::Camera const& cam);
        void                                        UpdateCache();

        NODISCARD const D_GRAPHICS::Viewport&       GetViewport() const { return mViewport; }
        NODISCARD const D_MATH::Rect&               GetScissorRect() const { return mScissorRect; }

        NODISCARD D_GRAPHICS::ViewportState         GetViewportState() const override;
        //NODISCARD VariableRateShadingState GetVariableRateShadingState() const override;
        //NODISCARD TextureSubresourceSet GetSubresources() const override;
        NODISCARD bool                              IsReverseDepth() const override;
        NODISCARD bool                              IsOrthographicProjection() const override;
        NODISCARD bool                              IsStereoView() const override;
        NODISCARD bool                              IsCubemapView() const override;
        NODISCARD bool                              IsBoxVisible(Darius::Math::Bounds::Aabb const& aabb) const override;
        NODISCARD bool                              IsMirrored() const override;
        NODISCARD D_MATH::Vector3                   GetViewOrigin() const override;
        NODISCARD D_MATH::Vector3                   GetViewDirection() const override;
        NODISCARD D_MATH::Vector3                   GetViewUpDirection() const override;
        NODISCARD D_MATH_CAMERA::Frustum            GetViewFrustum() const override;
        NODISCARD D_MATH_CAMERA::Frustum            GetWorldFrustum() const override;
        NODISCARD D_MATH::Matrix4                   GetViewMatrix() const override;
        NODISCARD D_MATH::Matrix4                   GetInverseViewMatrix() const override;
        NODISCARD D_MATH::Matrix4                   GetProjectionMatrix(bool includeOffset = true) const override;
        NODISCARD D_MATH::Matrix4                   GetInverseProjectionMatrix(bool includeOffset = true) const override;
        NODISCARD D_MATH::Matrix4                   GetViewProjectionMatrix(bool includeOffset = true) const override;
        NODISCARD D_MATH::Matrix4                   GetInverseViewProjectionMatrix(bool includeOffset = true) const override;
        NODISCARD D_MATH::Rect                      GetViewExtent() const override;
        NODISCARD D_MATH::Vector2                   GetPixelOffset() const override;
        NODISCARD float                             GetNearZ() const override;
        NODISCARD float                             GetFarZ() const override;

    protected:

        void                                        EnsureCacheIsValid() const;

        D_GRAPHICS::Viewport                        mViewport;
        D_MATH::Rect                                mScissorRect;
        //VariableRateShadingState                    mShadingRateState;
        D_MATH_CAMERA::Camera                       mCameraMath;

        D_MATH::Matrix4                             mPixelOffsetMatrix = D_MATH::Matrix4::Identity;
        D_MATH::Matrix4                             mInvPixelOffsetMatrix = D_MATH::Matrix4::Identity;
        D_MATH::Vector2                             mPixelOffset = D_MATH::Vector2::Zero;

        bool                                        mCacheValid = false;
    };

    class CompositeView : public ICompositeView
    {
    public:
        void                                        AddView(std::shared_ptr<IView> view);

        NODISCARD virtual uint32_t                  GetNumChildViews(ViewType::Enum supportedTypes) const override;
        NODISCARD virtual IView const*              GetChildView(ViewType::Enum supportedTypes, uint32_t index) const override;

    protected:
        D_CONTAINERS::DVector<std::shared_ptr<IView>>   mChildViews;
    };

    template<typename ChildType>
    class StereoView : public IView
    {
    public:
        NODISCARD D_GRAPHICS::ViewportState     GetViewportState() const override
        {
            auto rightState = mRightView.GetViewportState();
            auto leftState = mLeftView.GetViewportState();

            for (size_t i = 0; i < rightState.Viewports.size(); i++)
            {
                leftState.AddViewport(rightState.Viewports[i]);
            }

            for (size_t i = 0; i < rightState.ScissorRects.size(); i++)
            {
                leftState.AddScissorRect(rightState.ScissorRects[i]);
            }

            return leftState;
        }
        //NODISCARD VariableRateShadingState GetVariableRateShadingState() const override;
        //NODISCARD TextureSubresourceSet GetSubresources() const override;

        NODISCARD bool                              IsReverseDepth() const
        {
            return mLeftView.IsReverseDepth();
        }

        NODISCARD bool                              IsOrthographicProjection() const
        {
            return mLeftView.IsOrthographicProjection();
        }

        NODISCARD bool                              IsStereoView() const { return true; }
        NODISCARD bool                              IsCubemapView() const { return false; }

        NODISCARD bool                              IsBoxVisible(Darius::Math::Bounds::Aabb const& aabb) const
        {
            return mLeftView.IsBoxVisible(aabb) || mRightView.IsBoxVisible(aabb);
        }

        NODISCARD bool                              IsMirrored() const
        {
            return mLeftView.IsMirrored();
        }

        NODISCARD D_MATH::Vector3                   GetViewOrigin() const
        {
            (mLeftView.GetViewOrigin() + mRightView.GetViewOrigin()) * 0.5f;
        }

        NODISCARD D_MATH::Vector3                   GetViewDirection() const
        {
            return mLeftView.GetViewDirection();
        }

        NODISCARD D_MATH::Vector3                   GetViewUpDirection() const
        {
            return mLeftView.GetViewUpDirection();
        }

        NODISCARD D_MATH_CAMERA::Frustum            GetViewFrustum() const
        {
            /*auto leftFrustum = mLeftView.GetViewFrustum();
            auto rightFrustum = mRightView.GetViewFrustum();*/

            //leftFrustum.

            D_ASSERT_NOENTRY_M("Not implemented");
        }

        NODISCARD D_MATH_CAMERA::Frustum            GetWorldFrustum() const
        {
            D_ASSERT_NOENTRY_M("Not implemented");
        }
        
        NODISCARD D_MATH::Matrix4                   GetViewMatrix() const
        {
            D_ASSERT_NOENTRY();
            return D_MATH::Matrix4::Identity;
        }

        NODISCARD D_MATH::Matrix4                   GetInverseViewMatrix() const
        {
            D_ASSERT_NOENTRY();
            return D_MATH::Matrix4::Identity;
        }

        NODISCARD D_MATH::Matrix4                   GetProjectionMatrix(bool includeOffset = true) const
        {
            (void)includeOffset;
            D_ASSERT_NOENTRY();
            return D_MATH::Matrix4::Identity;
        }

        NODISCARD D_MATH::Matrix4                   GetInverseProjectionMatrix(bool includeOffset = true) const const
        {
            (void)includeOffset;
            D_ASSERT_NOENTRY();
            return D_MATH::Matrix4::Identity;
        }

        NODISCARD D_MATH::Matrix4                   GetViewProjectionMatrix(bool includeOffset = true) const const
        {
            (void)includeOffset;
            D_ASSERT_NOENTRY();
            return D_MATH::Matrix4::Identity;
        }

        NODISCARD D_MATH::Matrix4                   GetInverseViewProjectionMatrix(bool includeOffset = true) const const
        {
            (void)includeOffset;
            D_ASSERT_NOENTRY();
            return D_MATH::Matrix4::Identity;
        }

        NODISCARD D_MATH::Rect                      GetViewExtent() const
        {
            auto left = mLeftView.GetViewExtent();
            auto right = mRightView.GetViewExtent();
            return D_MATH::Rect::Union(left, right);
        }

        NODISCARD D_MATH::Vector2                   GetPixelOffset() const
        {
            return mLeftView.GetPixelOffset();
        }

        NODISCARD float                             GetNearZ() const
        {
            return mLeftView.GetNearZ();
        }

        NODISCARD float                             GetFarZ() const
        {
            return mRightView.GetFarZ();
        }

        NODISCARD uint32_t                          GetNumChildViews(ViewType::Enum supportedTypes) const
        {
            if (supportedTypes & ViewType::Stereo)
            {
                return 1u;
            }

            return 2u;
        }

        NODISCARD IView const*                      GetChildView(ViewType::Enum supportedTypes, uint32_t index) const
        {
            if(supportedTypes & ViewType::Stereo)
            { 
                D_ASSERT(index == 0);
                return this;
            }

            D_ASSERT(index < 2);

            if (index == 0)
                return &mLeftView;
            else
                return &mRightView;
        }

    protected:
        ChildType                               mLeftView;
        ChildType                               mRightView;

    private:
        typedef IView                           Super;
    };

    typedef StereoView<PlanarView> StereoPlanarView;

   /* class CubemapView : public IView
    {

    protected:
        D_MATH_CAMERA                           mCameraMath;
    };*/
}