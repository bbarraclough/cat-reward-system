#include <Arduino.h>
#include <Servo.h>
#define ENC_A 2
#define ENC_B 3
#define SERVO_PIN 9
#define INITIAL_TIME_THRES 3000
#define TREAT_COUNT_THRES 50
#define SEC_TO_MILLIS 1000
#define THRES_MULT_INCREMENT 0.05
#define TICK_TIME 250

// states of wheel
enum { spinClockwise,
	   spinCounterClockwise,
	   noSpin };
unsigned char wheelState = noSpin; // current state of wheel

// gambling addiction phases
enum { continuousReinforcement, // reward every time
	   fixedRatio,              // reward 1/3 times consistently
	   variable };              // introduce randomness
unsigned char gamblingState = continuousReinforcement;

volatile int counter = 0;
volatile unsigned int treatCounter = 0;  // for switching states and probability calculation
volatile double timeThresMultiplier = 1; // for linear increase in time threshold

Servo dispenserServo;
int dispenserServoPos = 0;

void read_encoder();
void determine_treat(long *timeSinceStateChange);
void dispense_treat();
double calculate_probability(long timeSinceStateChange);

void setup() {
	Serial.begin(115200);
	pinMode(ENC_A, INPUT_PULLUP);                                        // default HIGH so when there is connection to GND (turned) then can detect
	pinMode(ENC_B, INPUT_PULLUP);                                        // default HIGH so when there is connection to GND (turned) then can detect
	attachInterrupt(digitalPinToInterrupt(ENC_A), read_encoder, RISING); // whenever there is change on ENC_A pin, call read_encoder function
}

void loop() {

	static int lastCounter = 0;
	static int lastState = noSpin;
	static long timeSinceStateChange = millis();
	static long timeSinceLastCheck = millis();

	// if 100ms haven't passed since checking counter, do nothing
	if (millis() - timeSinceLastCheck < TICK_TIME) {
		return;
	}

	timeSinceLastCheck = millis();
	Serial.println(counter);

	// if count not changed, do nothing
	if (counter == lastCounter) {
		lastState = wheelState;
		wheelState = noSpin;
		// Serial.println(counter);
		return;
	}

	// print and update new counter
	Serial.println("Wheel Spinning");

	// update current and previous wheel states
	if (counter > lastCounter) {
		lastState = wheelState;
		wheelState = spinClockwise;
	} else {
		lastState = wheelState;
		wheelState = spinCounterClockwise;
	}

	// update last counter
	lastCounter = counter;

	// check wheel state
	if (wheelState != lastState) {
		// if wheel state just changed, save timestamp
		timeSinceStateChange = millis();
		return;
	} else {
		// dispense treat if wheel has been spinning for 1 second
		if (millis() - timeSinceStateChange > INITIAL_TIME_THRES) {
			determine_treat(&timeSinceStateChange);
		}
	}
}

// determines whether treat should be dispensed at threshold
void determine_treat(long *timeSinceStateChange) {
	switch (gamblingState) {
	// if at early stage, dispense treat. update state once threshold number of treats has been surpassed
	case continuousReinforcement:
		dispense_treat();
		*timeSinceStateChange = millis();
		if (treatCounter > TREAT_COUNT_THRES) {
			gamblingState = fixedRatio;
		}
		break;

	// if at middle state, dispense treat 1 in 3 times
	case fixedRatio:
		if (treatCounter % 3 == 0) {
			dispense_treat();
			*timeSinceStateChange = millis();
		}
		if (treatCounter > TREAT_COUNT_THRES * 2) {
			gamblingState = variable;
		}

	// if in late stage gambling, give treat based on probability function
	case variable:
		double probability = calculate_probability(*timeSinceStateChange);
		// "rolls" for treat
		if ((rand() % 101) / 100.0 < probability) {
			dispense_treat();
			*timeSinceStateChange = millis();
		}
	}
}

// function to reward longer running and reduce long periods without treat
// probability increases when: total count of treats dispensed is low,
// 							   wheel has been spinning for long time without treat
double calculate_probability(long timeSinceStateChange) {
	double timeSpinning = millis() - timeSinceStateChange;                                         // will be > current time threshold
	double timeFactor = (timeSpinning - INITIAL_TIME_THRES * timeThresMultiplier) / SEC_TO_MILLIS; // time spinning over the threshold in seconds

	// probability is asymptote to 1
	double prob = 1 - exp(-timeFactor / treatCounter);

	return prob;
}

// rewards with treat
void dispense_treat() {
	Serial.println("Treat Dispensed");
	treatCounter++;
	timeThresMultiplier += THRES_MULT_INCREMENT;
	dispenserServo.attach(SERVO_PIN);
	dispenserServo.write(90);
	delay(300);
	dispenserServo.write(0);
	delay(300);
	dispenserServo.detach();
}

void read_encoder() {

	if (digitalRead(ENC_B)) {
		counter--;
	} else {
		counter++;
	}

	/*
	// interrupt for both pins, updates counter if valid and rotated full indent

	static uint8_t old_AB = 3;                                                               // lookup table index (0000 0011 - where first 1 is A on, second 1 is B on)
	static int8_t encval = 0;                                                                // encoder val
	static const int8_t enc_states[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0}; // lookup table for encval increment, depending on prior and current state

	old_AB <<= 2; // shift 2 bits left

	// read current states of pin A and B
	if (digitalRead(ENC_A))
	    old_AB |= 0x02; // add current state of pin A (OR assign 0000 0010)
	if (digitalRead(ENC_B))
	    old_AB |= 0x01; // add current state of pin B (OR assign 0000 0001)

	encval += enc_states[(old_AB & 0x0f)]; // search and add lookup table result (& 0x0f is to only look at last 4 bits of old_AB)

	Serial.print("encval: ");
	Serial.println(encval);

	// once 4 steps detected (full indent), update counter accordingly
	if (encval > 1) {
	    counter++;
	    Serial.println("counter+");
	    Serial.println(counter);
	    encval = 0;
	} else if (encval < -1) {
	    counter--;
	    Serial.println("counter-");
	    Serial.println(counter);
	    encval = 0;
	}
	*/
}
