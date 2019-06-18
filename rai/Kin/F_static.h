/*  ------------------------------------------------------------------
    Copyright (c) 2017 Marc Toussaint
    email: marc.toussaint@informatik.uni-stuttgart.de

    This code is distributed under the MIT License.
    Please see <root-path>/LICENSE for details.
    --------------------------------------------------------------  */

#include "feature.h"

struct F_netForce : Feature {
  int i;               ///< which shapes does it refer to?
  double gravity=9.81;
  bool transOnly=false;

  F_netForce(int iShape, bool _transOnly=false, bool _zeroGravity=false);
  F_netForce(const rai::KinematicWorld& K, const char* iShapeName, bool _transOnly=false, bool _zeroGravity=false)
    : F_netForce(initIdArg(K,iShapeName), _transOnly, _zeroGravity){}
  
  virtual void phi(arr& y, arr& J, const rai::KinematicWorld& K);
  virtual uint dim_phi(const rai::KinematicWorld& K);
  
  virtual rai::String shortTag(const rai::KinematicWorld& K) { return STRING("static-" <<K.frames(i)->name); }
};


