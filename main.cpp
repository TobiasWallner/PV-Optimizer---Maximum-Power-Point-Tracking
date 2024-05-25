
extern "C"{

#include <DAVE.h>
#include <Interrupts.h>

}

int main(void)
{
  DAVE_STATUS_t status;

  status = DAVE_Init();           /* Initialization of DAVE APPs  */

  if(status == DAVE_STATUS_FAILURE)
  {
    /* Placeholder for error handler code. The while loop below can be replaced with an user error handler. */
    XMC_DEBUG("DAVE APPs initialization failed\n");

    while(1U)
    {

    }
  }

  /* ADC trigger signal setting */
  //XMC_CCU8_SLICE_SetTimerCompareMatchChannel2(PWM_CCU8_0.ccu8_slice_ptr,TRIGGER_ADC);
  //PWM_CCU8_0.ccu8_module_ptr->GCSS= 0x1;

  /* Start PWM */
  PWM_CCU8_Start(&PWM_Buck);
  PWM_CCU8_Start(&PWM_Boost);
  PWM_CCU8_SetDutyCycleSymmetric  (&PWM_Buck,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  30*100);
  PWM_CCU8_SetDutyCycleSymmetric  (&PWM_Boost,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  60*100);

  while(1U)
  {

  }
}

