#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdlib.h>
#include <string.h>

const char *spice_message_payload =
"69␟.MODEL MCH6001 NPN\n"
"+ IS=2.155f\n"
"+ BF=100.0\n"
"+ BF=122.0\n"
"+ NF=1.050\n"
"+ VAF=8.000\n"
"+ IKF=206.4m\n"
"+ ISE=1.384p\n"
"+ NE=2.278\n"
"+ BR=14.00\n"
"+ NR=1.042\n"
"+ VAR=4.000\n"
"+ IKR=360.0m\n"
"+ ISC=140.0f\n"
"+ NC=1.600\n"
"+ RB=2.000\n"
"+ IRB=1.500\n"
"+ RBM=25.00m\n"
"+ RE=450.0m\n"
"+ RC=1.200\n"
"+ XTB=0.000\n"
"+ EG=1.110\n"
"+ XTI=1.000\n"
"+ CJE=2.476p\n"
"+ VJE=750.0m\n"
"+ MJE=10.00m\n"
"+ TF=6.700p\n"
"+ XTF=50.00f\n"
"+ VTF=750.0m\n"
"+ ITF=50.00\n"
"+ PTF=200.0m\n"
"+ CJC=175.0f\n"
"+ VJC=200.0f\n"
"+ MJC=1.150p\n"
"+ XCJC=1.000\n"
"+ TR=0.000\n"
"+ FC=500.0m\n"
"+ CJS=550.0f\n"
"+ VJS=150.0m\n"
"+ MJS=136.0m\n"
"\n"
"Q1 N003 N001 N004 0 MCH6001\n"
"Q2 N004 N006 N007 0 MCH6001\n"
"V§POSITIVE_SUPPLY V+ 0 5\n"
"R§EMITTER_RESISTOR N007 0 ${REMITTER}\n"
"L§OUTPUT_CHOKE V+ N003 ${LOUT}\n"
"C§OUTPUT_DC_BLOCK VOUT N003 ${COUT}\n"
"R2 VOUT 0 50\n"
"L§INPUT_CHOKE N002 N006 ${LIN}\n"
"C§INPUT_DC_BLOCK N006 N005 ${CIN}\n"
"V4 VIN 0 0 AC 0.01\n"
"R§INPUT_IMPEDANCE N005 VIN 50\n"
"R§BALLAST V+ N001 ${RBALLAST}\n"
"R§COMMON_BASE_DIVIDER N001 N002 ${RCOMMONBASE}\n"
"R§EMITTER_FOLLOWER_DIVIDER N002 0 ${REMITTERFOLLOWER}\n"
"C§BASE_BYPASS N001 0 ${CBYPASS}";

int main(int argc, char *argv[]) {
  assert(argc == 3);

  char *submission_message_queue_name = argv[1];
  char *receive_message_queue_name = argv[2];

  mqd_t send_mq = mq_open(submission_message_queue_name, O_WRONLY);
  mqd_t receive_mq = mq_open(receive_message_queue_name, O_RDONLY);
  
  mq_send(
    send_mq, 
    spice_message_payload, 
    strlen(spice_message_payload) + 1, 
    0
  );
  

  struct mq_attr receive_attributes;
  mq_getattr(receive_mq, &receive_attributes);
  char *received_message = malloc(receive_attributes.mq_msgsize);
  mq_receive(
    receive_mq, 
    received_message, 
    receive_attributes.mq_msgsize, 
    NULL
  );
  


}