#include "PID_AutoTune_v0.h"

#include <stdio.h>
#include <math.h>

PID_ATune::PID_ATune(double* Input, double* Output, double* Setpoint, unsigned long* Millis, int ControllerDirection)
{
	input = Input;
	output = Output;
	setpoint = Setpoint;
	millis = Millis;
	controlType = PI_CONTROL ; //default to PI (not PID)
	noiseBand = 0.5;
	running = false;
	oStep = 30;
	SetSampleTime(2);
	SetLookbackTime(10);
	peakCount = 0;
	SetControllerDirection(ControllerDirection);
}

void PID_ATune::Cancel()
{
	running = false;
}

int PID_ATune::Runtime()
{
	justevaled=false;
	if(peakCount>=10 && running) // filled peaks array and didn't find peaks within tolerance so end now anyway
	{
		running = false;
		FinishUp();
		return 1;
	}
	unsigned long now = *millis;

	if(running && (now-lastTime)<sampleTime) return 2; // called before sample time elapsed
	lastTime = now;
	double refVal = *input;
	justevaled=true;
	if(!running)
	{ //initialize working variables the first time around
	  // assumption seems to be the first input sample is at steady state for the setpoint and output to the controller
		peakType = 0; //first time indicator => 0, then max => +1, min => -1
		peakCount=0;
		peak1 = now;
		justchanged=false;
		absMax=refVal;
		absMin=refVal;
		running = true;
		outputStart = *output;
		*output = outputStart + (controllerDirection==DIRECT ? oStep: -oStep); // assure output bumped initially; it's done below also but only if outside noise band
		inputCount = 0;
	}
	else
	{
		if(refVal>absMax)absMax=refVal;
		if(refVal<absMin)absMin=refVal;
	}

	//oscillate the output base on the input's relation to the setpoint

	if(refVal>*setpoint+noiseBand) *output = outputStart - (controllerDirection==DIRECT ? oStep: -oStep);
	else if (refVal<*setpoint-noiseBand) *output = outputStart + (controllerDirection==DIRECT ? oStep: -oStep);

	if(inputCount < nLookBack)
	{  //we don't want to trust the maxes or mins until the inputs array has been filled; [nLookBack-1]=oldest, nLookBack[0]=newest
	lastInputs[nLookBack-1-inputCount++] = refVal;
	return 0;
	}

  //id peaks

  isMax=true;
  isMin=true;

  for(int i=nLookBack-1;i>=0;i--)
  {  //determine if current the max or min relative to all the nLookBack samples in lastInputs?
    double val = lastInputs[i];
    if(isMax) isMax = refVal>val;
    if(isMin) isMin = refVal<val;
    lastInputs[i+1] = lastInputs[i]; //move the last inputs up 1 place and make room for the new in [0]
  }
  lastInputs[0] = refVal;

  if(isMax) //this is max compared to nLookBack samples
  {
    if(peakType==0)peakType=1; //this is max on first time
    if(peakType==-1) //this is max coming from a min
    {
      peakType = 1;
      peak2 = peak1; //save the time of the previous max peak; assumes the first peak found is a max otherwise peak1 is initialized at now and peak1==peak2 -- but so what;
    }
    peak1 = now; //record the time of the current max peak
    peaks[peakCount] = refVal; //first peakCount is 0 then bumped by each min coming from a max - see below

  }
  else if(isMin)
  {
    if(peakType==0)peakType=-1; //this is min on first time
    if(peakType==1) //this is min coming from a max
    {
      peakType=-1;
      peakCount++;
      justchanged=true;
	}

    if(peakCount<10)peaks[peakCount] = refVal;
  }

  if(justchanged && peakCount>2) //2 periods used if count is 3; can increase to say 7 to oscillate more often before checking (note that peaks has [10] elements capacity unless changed)
  { //we've transitioned.  check if we can autotune based on the last peaks being consistent
    double avgSeparation = (fabs(peaks[peakCount-1]-peaks[peakCount-2])+fabs(peaks[peakCount-2]-peaks[peakCount-3]))/2;
    if( avgSeparation < 0.20*(absMax-absMin))  // the constant fraction to tolerate for tuning is arbitrary; may need adjusting
	{
	  if(peakCount>7)
	  {
		  running = false;
		  FinishUp();
		  return 1;
	  }
	  else
	  {
		  SetKuPu();
		  justchanged=false;
	  	  return 3;
	  }
	}
  }
  justchanged=false;
  return 0;
}

void PID_ATune::FinishUp()
{
	  *output = outputStart;
	  SetKuPu();
}

void PID_ATune::SetKuPu()
{
      //we can generate tuning parameters!
      Ku = 4*(2*oStep)/((absMax-absMin)*3.14159);
      Pu = (double)(peak1-peak2) / 1000.L;  // compute period and convert from millisecs to secs
}

double PID_ATune::GetPeak_1()
{
	return (double)peak1/1000.L;
}

double PID_ATune::GetPeak_2()
{
	return (double)peak2/1000.L;
}

double PID_ATune::GetKp()
{
	return controlType==PID_CONTROL ? 0.6 * Ku : 0.4 * Ku;
}

double PID_ATune::GetKi()
{
	return controlType==PID_CONTROL ? 1.2*Ku / Pu : 0.48 * Ku / Pu;  // Ki = Kc/Ti
}

double PID_ATune::GetKd()
{
	return controlType==PID_CONTROL ? 0.075 * Ku * Pu : 0;  //Kd = Kc * Td
}

double PID_ATune::GetKu()
{
	return Ku;
}

double PID_ATune::GetPu()
{
	return Pu;
}

void PID_ATune::SetOutputStep(double Step)
{
	oStep = Step;
}

double PID_ATune::GetOutputStep()
{
	return oStep;
}

void PID_ATune::SetControlType(int Type) // 0=PI_CONTROL, 1=PID_CONTROL
{
	controlType = Type;
}

int PID_ATune::GetControlType()
{
	return controlType;
}

void PID_ATune::SetNoiseBand(double Band)
{
	noiseBand = Band;
}

double PID_ATune::GetNoiseBand()
{
	return noiseBand;
}

/* SetSampleTime(...) *********************************************************
 * sets the period, in Milliseconds, at which the calculation is performed
 ******************************************************************************/
void PID_ATune::SetSampleTime(int NewSampleTime)
{
   if (NewSampleTime > 0)
   {
	sampleTime = (unsigned long)NewSampleTime;
   }

}

// milliseconds to look back for min/max;
// user must make sure not to have longer than 1 period of the process variable oscillation
// limited to 1 to 100 samples without notification!!!!!
void PID_ATune::SetLookbackTime(int value)
{
	nLookBack = value / sampleTime;

    if (nLookBack<1) nLookBack = 1;

	if(nLookBack > 100) nLookBack = 100;

	return;
}

int PID_ATune::GetLookbackTime()
{
	return nLookBack * sampleTime;
}
/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void PID_ATune::SetControllerDirection(int Direction)
{
   controllerDirection = Direction;
}
