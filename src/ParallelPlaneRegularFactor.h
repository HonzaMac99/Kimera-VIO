/*
 * @file RegularPlane3Factor.cpp
 * @brief RegularPlane3 Factor class
 * @author Antoni Rosinol
 * @date February 20, 2018
 */

#pragma once

#include <gtsam/geometry/OrientedPlane3.h>
#include <gtsam/nonlinear/NonlinearFactor.h>

namespace gtsam {

/**
 * Factor to implement error between a point landmark and a plane.
 */
class ParallelPlaneRegularFactor: public NoiseModelFactor2<OrientedPlane3,
                                                           OrientedPlane3> {
protected:
  Key plane1Key_;
  Key plane2Key_;
  std::string type_;

  typedef NoiseModelFactor2<OrientedPlane3, OrientedPlane3> Base;

public:

  /// Constructor
  ParallelPlaneRegularFactor() {
  }
  virtual ~ParallelPlaneRegularFactor() {}

  ParallelPlaneRegularFactor(const Key& plane1Key, const Key& plane2Key,
                             const SharedGaussian& noiseModel) :
                                        Base(noiseModel, plane1Key, plane2Key),
                                        plane1Key_(plane1Key),
                                        plane2Key_(plane2Key),
                                        type_("None"){}

  /// print
  void print(const std::string& s = "ParallelPlaneRegularFactor",
      const KeyFormatter& keyFormatter = DefaultKeyFormatter) const;

  /// evaluateError
  /// Hplane1: jacobian of h wrt plane1
  /// Hplane2: jacobian of h wrt plane2
  Vector evaluateError(
                    const OrientedPlane3& plane_1,
                    const OrientedPlane3& plane_2,
                    boost::optional<Matrix&> H_plane_1 = boost::none,
                    boost::optional<Matrix&> H_plane_2 = boost::none) const {
      return doEvaluateError(plane_1, plane_2, H_plane_1, H_plane_2);
  }

private:
  //// See non-virtual interface idiom (NVI idiom) for reasons to do this.
  /// Basically it's to avoid re-defining default parameters in derived classes.
  /// See also Item 37 of Effective_C++.
  virtual Vector doEvaluateError(
                   const OrientedPlane3& plane_1,
                   const OrientedPlane3& plane_2,
                   boost::optional<Matrix&> H_plane_1,
                   boost::optional<Matrix&> H_plane_2) const = 0;
};

class ParallelPlaneRegularTangentSpaceFactor: public ParallelPlaneRegularFactor {
public:
  /// Constructor
  ParallelPlaneRegularTangentSpaceFactor() {
  }
  virtual ~ParallelPlaneRegularTangentSpaceFactor() {}

  ParallelPlaneRegularTangentSpaceFactor(const Key& plane1Key,
                                         const Key& plane2Key,
                                         const SharedGaussian& noiseModel) :
                ParallelPlaneRegularFactor(plane1Key, plane2Key, noiseModel) {
      this->type_ = "Tangent Space";
  }

private:
  /// evaluateError
  /// Hplane1: jacobian of h wrt plane1
  /// Hplane2: jacobian of h wrt plane2
  virtual Vector doEvaluateError(
                       const OrientedPlane3& plane_1,
                       const OrientedPlane3& plane_2,
                       boost::optional<Matrix&> H_plane_1,
                       boost::optional<Matrix&> H_plane_2) const {
    Unit3 plane_normal_1 = plane_1.normal();
    Unit3 plane_normal_2 = plane_2.normal();
    Matrix22 H_n_1, H_n_2;
    Vector2 err;
    err =  plane_normal_1.errorVector(plane_normal_2, H_n_1, H_n_2);
    if (H_plane_1) {
      *H_plane_1 = H_n_1, Vector2::Zero();
    }
    if (H_plane_2) {
      *H_plane_2 = H_n_2, Vector2::Zero();
    }
    return (err);
  }
};

class ParallelPlaneRegularBasicFactor: public ParallelPlaneRegularFactor {
public:
  /// Constructor
  ParallelPlaneRegularBasicFactor() {
  }
  virtual ~ParallelPlaneRegularBasicFactor() {}

  /// Constructor with measured plane coefficients (a,b,c,d), noise model, pose symbol
  ParallelPlaneRegularBasicFactor(const Key& plane1Key,
                                         const Key& plane2Key,
                                         const SharedGaussian& noiseModel) :
                ParallelPlaneRegularFactor(plane1Key, plane2Key, noiseModel) {
      this->type_ = "Tangent Space";
  }

private:
  /// evaluateError
  /// Hplane1: jacobian of h wrt plane1
  /// Hplane2: jacobian of h wrt plane2
  virtual Vector doEvaluateError(
                       const OrientedPlane3& plane_1,
                       const OrientedPlane3& plane_2,
                       boost::optional<Matrix&> H_plane_1,
                       boost::optional<Matrix&> H_plane_2) const {
    Unit3 plane_normal_1 = plane_1.normal();
    Unit3 plane_normal_2 = plane_2.normal();
    Vector3 err (0,0,0);
    err =  Vector3(plane_normal_1.point3().x() - plane_normal_2.point3().x(),
                   plane_normal_1.point3().y() - plane_normal_2.point3().y(),
                   plane_normal_1.point3().z() - plane_normal_2.point3().z());
    if (H_plane_1) {
      // Jacobian of plane retraction when v = Vector3::Zero(), to speed-up
      // computations.
      *H_plane_1 = plane_normal_1.basis(), Vector3::Zero();
    }
    if (H_plane_2) {
      // Jacobian of plane retraction when v = Vector3::Zero(), to speed-up
      // computations.
      *H_plane_2 = -plane_normal_2.basis(), Vector3::Zero();
    }
    return (err);
  }
};

} // End of gtsam namespace.
