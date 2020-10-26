/*  ------------------------------------------------------------------
    Copyright (c) 2011-2020 Marc Toussaint
    email: toussaint@tu-berlin.de

    This code is distributed under the MIT License.
    Please see <root-path>/LICENSE for details.
    --------------------------------------------------------------  */

#pragma once

#include "CtrlObjective.h"
#include <Kin/feature.h>

//===========================================================================

struct CtrlSet {
  rai::Array<shared_ptr<CtrlObjective>> objectives;    ///< list of objectives

  shared_ptr<CtrlObjective> addObjective(const ptr<Feature>& f, ObjectiveType type, double transientStep=-1.);
  shared_ptr<CtrlObjective> add_qControlObjective(uint order, double scale, const rai::Configuration& C);

  operator rai::Array<shared_ptr<CtrlObjective>>&(){ return objectives; }

  void report(ostream& os=cout) const;

  bool canBeInitiated(const rai::Configuration& Ctuple) const;
  bool isConverged(const rai::Configuration& Ctuple) const;
};

//===========================================================================

bool isFeasible(const CtrlSet& CS, const rai::Configuration& Ctuple, bool initOnly=true, double eqPrecision=1e-4);

CtrlSet operator+(const CtrlSet& A, const CtrlSet& B);
