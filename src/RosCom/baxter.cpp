#include "baxter.h"

#ifdef MLR_ROS

#include "roscom.h"
#include <baxter_core_msgs/HeadPanCommand.h>
#include <baxter_core_msgs/EndEffectorCommand.h>
#include <baxter_core_msgs/JointCommand.h>

struct sSendPositionCommandsToBaxter{
  ros::NodeHandle nh;
  ros::Publisher pubL, pubR, pubHead, pubGripper;
  ors::KinematicWorld baxterModel;
};

baxter_core_msgs::JointCommand conv_qRef2baxterMessage(const arr& q_ref, const ors::KinematicWorld& baxterModel, const char* prefix){
  baxter_core_msgs::JointCommand msg;
  msg.mode = 1;
  for(ors::Joint *j:baxterModel.joints) if(j->name.startsWith(prefix)){
    msg.command.push_back(q_ref(j->qIndex));
    msg.names.push_back(j->name.p);
  }
  return msg;
}

bool baxter_update_qReal(arr& qReal, const sensor_msgs::JointState& msg, const ors::KinematicWorld& baxterModel){
  uint n = msg.name.size();
  if(!n) return false;
  for(uint i=0;i<n;i++){
    ors::Joint *j = baxterModel.getJointByName(msg.name[i].c_str(), false);
    if(j) qReal(j->qIndex) = msg.position[i];
  }
  return true;
}

baxter_core_msgs::HeadPanCommand getHeadMsg(const arr& q_ref, const ors::KinematicWorld& baxterModel){
  baxter_core_msgs::HeadPanCommand msg;
  ors::Joint *j = baxterModel.getJointByName("head_pan");
  msg.target = q_ref(j->qIndex);
  msg.speed_ratio = 1.;
  return msg;
}

baxter_core_msgs::EndEffectorCommand getGripperMsg(const arr& q_ref, const ors::KinematicWorld& baxterModel){
  baxter_core_msgs::EndEffectorCommand msg;
  ors::Joint *j = baxterModel.getJointByName("l_gripper_l_finger_joint");
  mlr::String str;
  str <<"{ \"position\":" <<1000.*q_ref(j->qIndex) <<", \"dead zone\":5.0, \"force\": 40.0, \"holding force\": 30.0, \"velocity\": 50.0 }";
  cout <<str <<endl;

  msg.id = 65538;
  msg.command = msg.CMD_GO;
  msg.args = str.p;
  msg.sender = "foo";
  msg.sequence = 1;
  return msg;
}

SendPositionCommandsToBaxter::SendPositionCommandsToBaxter()
  : Module("SendPositionCommandsToBaxter"),
    q_ref(this, "q_ref", true),
    s(NULL){
}

void SendPositionCommandsToBaxter::open(){
  s = new sSendPositionCommandsToBaxter;
  s->pubR = s->nh.advertise<baxter_core_msgs::JointCommand>("robot/limb/right/joint_command", 1);
  s->pubL = s->nh.advertise<baxter_core_msgs::JointCommand>("robot/limb/left/joint_command", 1);
  s->pubHead = s->nh.advertise<baxter_core_msgs::HeadPanCommand>("robot/head/command_head_pan", 1);
  s->pubGripper = s->nh.advertise<baxter_core_msgs::EndEffectorCommand>("robot/end_effector/left_gripper/command", 1);
  s->baxterModel.init(mlr::mlrPath("data/baxter_model/baxter-modifications.ors").p);
}

void SendPositionCommandsToBaxter::step(){
  s->pubR.publish(conv_qRef2baxterMessage(q_ref.get(), s->baxterModel, "right_"));
  s->pubL.publish(conv_qRef2baxterMessage(q_ref.get(), s->baxterModel, "left_"));
  s->pubHead.publish(getHeadMsg(q_ref.get(), s->baxterModel));
  s->pubGripper.publish(getGripperMsg(q_ref.get(), s->baxterModel));
}

void SendPositionCommandsToBaxter::close(){
  delete s;
}

#endif