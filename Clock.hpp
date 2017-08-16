#ifndef INC_MYCLOCK_H
#define INC_MYCLOCK_H

#include <chrono>


// Clock

class Clock
{

private:
	bool state_;
	std::chrono::time_point<std::chrono::system_clock> start_time_;
	std::chrono::time_point<std::chrono::system_clock> pause_time_;
	std::chrono::time_point<std::chrono::system_clock> now_;
	std::chrono::time_point<std::chrono::system_clock> previous_time_;
	long long unsigned int elapsed_time_;
	int tempo_;
	unsigned int ticks_per_beat_, beats_per_bar_;
	float metric_grain_, tick_duration_, time_delta_, time_dust_;
	unsigned int bar_, beat_, tick_;
	unsigned int hour_, minute_, second_;
	long unsigned int previous_ticks_, now_ticks_, elapsed_ticks;

public:
	Clock();
	~Clock();
	void metrics();
	void start();
	void pause();
	void stop();
	void rewind();
	inline bool getState() { return state_; }
	inline bool atZero() { return ( time_delta_ == 0 ); }
	inline bool isStarted() { return state_; }
	inline int * getTempoP() { return &tempo_; }
	inline unsigned int getHour() { return hour_; }
	inline unsigned int getMinute() { return minute_; }
	inline unsigned int getSecond() { return second_; }
	inline unsigned int getBar() { return bar_; }
	inline unsigned int getBeat() { return beat_; }
	inline unsigned int getTick() { return tick_; }
	inline unsigned int getBeatsPerBar() { return beats_per_bar_; }
	inline unsigned int getTicksPerBeat() { return ticks_per_beat_; }
	void setTicksPerBeat( unsigned int tpb );
	unsigned int update();

};

#endif
