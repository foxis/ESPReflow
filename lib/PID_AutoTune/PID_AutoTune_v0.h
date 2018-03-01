#ifndef PID_AutoTune_v0
#define PID_AutoTune_v0

// control types:
#define PI_CONTROL 0
#define PID_CONTROL 1
#define DIRECT  0
#define REVERSE  1

class PID_ATune
{
  public:
  //commonly used functions **************************************************************************
	PID_ATune(double* Input, double* Output, double* Setpoint, unsigned long* Millis, int ControllerDirection);
												// * Constructor

    int Runtime();						   		// * Similar to the PID Compute function, returns:
    											// 0 a time step toward tuning has been completed okay
    											// 1 tuning completed for better or for worse; no longer running tuning
    											// 2 called too quickly; time step has not elapsed; no action taken
    											// 3 a peak was found and tuning parameters are available; additional tuning attempts will be made

    void Cancel();								// * Stops the AutoTune

	void SetOutputStep(double);					// * how far above and below the starting value will the output step?
	double GetOutputStep();						//

	void SetControlType(int); 					// * Determines if the tuning parameters returned will be PI (D=0)
	int GetControlType();						//   or PID.  (0=PI, 1=PID)

	void SetLookbackTime(int);					// * how far back are we looking to identify peaks
	int GetLookbackTime();						//

	void SetSampleTime(int);					// * millisec

	void SetNoiseBand(double);					// * the autotune will ignore signal chatter smaller than this value
	double GetNoiseBand();						//   this should be accurately set

	double GetKp();								// * once autotune is complete (Runtime returns 1 or 3),
	double GetKi();								//   these functions contain the computed tuning parameters.
	double GetKd();								//
	double GetKu();								//
	double GetPu();								//
	double GetPeak_1();							// * time last maximum found
	double GetPeak_2();							// * time 2nd to the last maximum found
	void SetControllerDirection(int);	 		// * Sets the Direction, or "Action" of the controller. DIRECT
										  	  	//   means the output will increase when error is positive. REVERSE
										  	  	//   means the opposite.  it's very unlikely that this will be needed
										  	  	//   once it is set in the constructor.
  private:
    void FinishUp();
    void SetKuPu();
	bool isMax, isMin;
	double *input, *output, *setpoint;
	unsigned long *millis; // System time in milliseconds
	double noiseBand;
	int controlType;
	bool running;
	unsigned long peak1, peak2, lastTime;
	unsigned long sampleTime;
	int nLookBack;
	int controllerDirection;
	int peakType;
	double lastInputs[101];
	int inputCount;
    double peaks[10];
	int peakCount;
	bool justchanged;
	bool justevaled;
	double absMax, absMin;
	double oStep;
	double outputStart;
	double Ku, Pu;
};
#endif
