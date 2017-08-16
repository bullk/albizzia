#include <spdlog/spdlog.h>
#include "Clock.hpp"
//#include "Waiter.hpp"


Clock::Clock ()
{
	state_ = false;
	tempo_ = 120;
	ticks_per_beat_ = 120;
	beats_per_bar_ = 4;
	metrics();
	rewind();
}

Clock::~Clock ()
{}

void Clock::metrics()
{
	metric_grain_ = 60.0f / ticks_per_beat_;
	spdlog::get( "main" )->info( "metric grain : {}", metric_grain_ );
}

void Clock::start()
{
	if ( not(state_) )
	{
		previous_time_ = std::chrono::system_clock::now();
		start_time_ = start_time_ + ( previous_time_ - pause_time_);
		pause_time_ = start_time_;
		state_ = true;
	}
}

void Clock::pause()
{
	if (state_)
	{
		pause_time_ = std::chrono::system_clock::now();
		state_ = false;
	}
}

void Clock::stop ()
{
	state_ = false;
	rewind ();
}
	
void Clock::rewind ()
{
	bar_ = 0;
	beat_ = 0;
	tick_ = 0;
	hour_ = 0;
	minute_ = 0;
	second_ = 0;
	elapsed_time_ = 0;
	elapsed_ticks = 0;
	previous_ticks_ = 0;
	now_ticks_ = 0;
	time_delta_ = 0;
	time_dust_ = 0;
}
	
void Clock::setTicksPerBeat( unsigned int tpb )
{
	ticks_per_beat_ = tpb;
	metrics();
	rewind();
}

unsigned int Clock::update ()
{
	unsigned int midi_ticks = 0;
	if (state_) {
		auto mainlog = spdlog::get( "main" );
		mainlog->debug( "tempo_ = {}", tempo_ );
		tick_duration_ = metric_grain_ / tempo_;
		mainlog->debug( "tick_duration_ = {}", tick_duration_);
		float tdus = 1000000 * tick_duration_;
		mainlog->debug( "tdus = {}", tdus );
		unsigned int ticks_delta = 0;
		
		now_ = std::chrono::system_clock::now();
		//mainlog->debug( "now_ = {}", now_ );
		//time_elastic_ = std::chrono::duration_cast<std::chrono::microseconds> (now_ - start_time_).count();
		time_delta_ = std::chrono::duration_cast<std::chrono::microseconds> (now_ - previous_time_).count() + time_dust_;
		mainlog->debug( "time_delta_ = {}", time_delta_ );
		//ticks_delta = now_ticks_ - previous_ticks_;
		ticks_delta = (unsigned int) ( time_delta_ / tdus );
		mainlog->debug( "ticks_delta = {}", ticks_delta );
		float x = ticks_delta * tdus;
		mainlog->debug( "x = {}", x );
		time_dust_ = time_delta_ - x;
		mainlog->debug( "time_dust_ = {}", time_dust_ );
		//mainlog->debug( " = {}",  );
		
		// h:m:s
		elapsed_time_ += (long long unsigned int) x;
		mainlog->debug( "elapsed_time_ = {}", elapsed_time_ );
		int tot_s = (int) (elapsed_time_ / 1000000);
		int tot_m = tot_s / 60;
		hour_ = tot_m / 60;
		minute_ = tot_m % 60;
		second_ = tot_s % 60;

		// MIDI (bar_, beat_, tick_)
		//unsigned int previous_bar = bar_;
		//unsigned int previous_beat = beat_;
		elapsed_ticks += ticks_delta;
		mainlog->debug( "elapsed_ticks = {}", elapsed_ticks );
		//now_ticks_ = (long unsigned int) (time_delta_ / (1000000 * tick_duration_));
		unsigned int nbeats = (unsigned int)(elapsed_ticks / ticks_per_beat_);
		tick_ = (unsigned int) (elapsed_ticks % ticks_per_beat_);
		beat_ = 1 + nbeats % beats_per_bar_;
		//if ( previous_beat != beat_ ) Waiter::getInstance () -> beat ();
		bar_ = 1 + nbeats / beats_per_bar_;
		//if ( previous_bar != bar_ ) Waiter::getInstance () -> bar ();

		// how many MIDI ticks ?
		if (now_ticks_ < previous_ticks_)
			previous_ticks_ = now_ticks_;
		if ( (previous_ticks_ == 0) and (ticks_delta == 0) ) midi_ticks = 1;
		if ( ticks_delta > 0 ) midi_ticks = ticks_delta;

		// MIDI Clock update
		previous_ticks_ = now_ticks_;
		previous_time_ = now_;

		// DEBUG
		//char bbt[13];
		//sprintf (bbt, "%02d:%02d:%03d   ", daClock -> getBar(), daClock -> getBeat(), diApp.clock -> getTick());
		//std::cout << bbt << midi_ticks << std::endl;
	}
	return midi_ticks;
}

