/*  ------------------------------------------------------------------
    Copyright (c) 2011-2020 Marc Toussaint
    email: toussaint@tu-berlin.de

    This code is distributed under the MIT License.
    Please see <root-path>/LICENSE for details.
    --------------------------------------------------------------  */

#include "CtrlProblem.h"

#include "CtrlTargets.h"

CtrlProblem::CtrlProblem(rai::Configuration& _C, double _tau, uint k_order)
  : tau(_tau) {
  komo.setModel(_C, true);
  komo.setTiming(1., 1, _tau, k_order);
  komo.setupConfigurations2();
}

CtrlProblem::~CtrlProblem(){
}

std::shared_ptr<CtrlObjective> CtrlProblem::add_qControlObjective(uint order, double scale, const arr& target) {
  ptr<Objective> o = komo.add_qControlObjective({}, order, scale, target);
  return addObjective(o->feat, NoStringA, o->type);
}

void CtrlProblem::set(const rai::Array<ptr<CtrlObjective>>& O) {
  objectives.clear();
  objectives.resize(O.N);
  for(uint i=0;i<O.N;i++) objectives(i) = O(i).get();
}

void CtrlProblem::addObjectives(const rai::Array<ptr<CtrlObjective>>& O) {
  for(auto& o:O) {
    objectives.append(o.get());
  }
}

void CtrlProblem::delObjectives(const rai::Array<ptr<CtrlObjective>>& O) {
  for(auto& o:O) {
    objectives.removeValue(o.get());
  }
}

ptr<CtrlObjective> CtrlProblem::addObjective(const ptr<Feature>& _feat, const StringA& frames, ObjectiveType _type) {
  std::shared_ptr<CtrlObjective> t = make_shared<CtrlObjective>();
  t->feat = _feat;
  t->type = _type;
  if(!!frames && frames.N){
    if(frames.N==1 && frames.scalar()=="ALL") t->feat->frameIDs = framesToIndices(komo.world.frames); //important! this means that, if no explicit selection of frames was made, all frames (of a time slice) are referred to
    else t->feat->frameIDs = komo.world.getFrameIDs(frames);
  }
  addObjectives({t});
  return t;
}

ptr<CtrlObjective> CtrlProblem::addObjective(const FeatureSymbol& feat, const StringA& frames, ObjectiveType type, const arr& scale, const arr& target, int order) {
  return addObjective(symbols2feature(feat, frames, komo.world, scale, target, order), NoStringA, type);
}

void CtrlProblem::update(rai::Configuration& C) {
  //-- update the KOMO configurations (push one step back, and update current configuration)
  for(int t=-komo.k_order; t<0; t++) {
    komo.setConfiguration(t, komo.getConfiguration_q(t+1));
  }
  arr q = C.getJointState();
  komo.setConfiguration(-1, q);
  komo.setConfiguration(0, q);
  komo.pathConfig.ensure_q();

  //-- step the moving targets forward, if they have one
  for(CtrlObjective* o: objectives) if(o->active){
    if(!o->name.N) o->name = o->feat->shortTag(C);

    if(o->movingTarget) {
      o->y_buffer = o->getValue(*this);
      ActStatus s_new = o->movingTarget->step(tau, o, o->y_buffer);
      if(o->status != s_new) {
        o->status = s_new;
        //callbacks
      }
    } else {
      if(o->status != AS_running) {
        o->status = AS_running;
        //callbacks
      }
    }
  }
}

void CtrlProblem::report(std::ostream& os) {
  os <<"    control objectives:" <<endl;
  for(CtrlObjective* o: objectives) o->reportState(os);
  os <<"    optimization result:" <<endl;
  os <<optReport <<endl;
}

static int animate=0;

arr CtrlProblem::solve() {
#if 0
  TaskControlMethods M(komo.getConfiguration_t(0).getHmetric());
  arr q = komo.getConfiguration_t(0).getJointState();
  q += M.inverseKinematics(objectives, NoArr, {});
  return q;
#elif 1
  komo.clearObjectives();
  for(CtrlObjective* o: objectives) if(o->active){
    komo.addObjective({}, o->feat, {}, o->type);
  }
  OptOptions opt;
  opt.stopTolerance = 1e-4;
  opt.stopGTolerance = 1e-4;
  opt.stopIters = 20;
//  opt.nonStrictSteps=-1;
  opt.maxStep = .1; //*tau; //maxVel*tau;
//  opt.damping = 1.;
//  opt.maxStep = 1.;
//  komo.verbose=4;
  komo.animateOptimization=animate;
  komo.optimize(0., opt);
  optReport = komo.getReport(false);
  if(optReport.get<double>("sos")>.1){
      cout <<optReport <<endl <<"something's wrong?" <<endl;
      rai::wait();
      animate=2;
  }
  return komo.getPath_q(0);
#else
  return solve_optim(*this);
#endif
}
