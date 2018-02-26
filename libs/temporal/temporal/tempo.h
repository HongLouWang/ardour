/*
    Copyright (C) 2017 Paul Davis

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __temporal_tempo_h__
#define __temporal_tempo_h__

#include <list>
#include <string>
#include <vector>
#include <cmath>
#include <exception>

#include <glibmm/threads.h>

#include "pbd/enum_convert.h"
#include "pbd/signals.h"
#include "pbd/statefuldestructible.h"

#include "temporal/visibility.h"
#include "temporal/beats.h"
#include "temporal/bbt_time.h"
#include "temporal/superclock.h"
#include "temporal/timeline.h"
#include "temporal/types.h"

namespace Temporal {

class Meter;
class TempoMap;

/** Tempo, the speed at which musical time progresses (BPM).
 */

class LIBTEMPORAL_API Tempo {
  public:
	enum Type {
		Ramped,
		Constant
	};

	static std::string xml_node_name;

	Tempo (XMLNode const &);

	/**
	 * @param npm Note Types per minute
	 * @param note_type Note Type (default `4': quarter note)
	 */
	Tempo (double npm, int note_type = 4) 
		: _superclocks_per_note_type (double_npm_to_sc (npm))
		, _end_superclocks_per_note_type (double_npm_to_sc (npm))
		, _note_type (note_type)
		, _active (true)
		, _locked_to_meter (false)
		, _clamped (false)
		, _type (Tempo::Constant) {}

	Tempo (double npm, double enpm, int note_type = 4)
		: _superclocks_per_note_type (double_npm_to_sc (npm))
		, _end_superclocks_per_note_type (double_npm_to_sc (enpm))
		, _note_type (note_type)
		, _active (true)
		, _locked_to_meter (false)
		, _clamped (false)
		, _type (npm != enpm ? Tempo::Ramped : Tempo::Constant) {}

	/* these five methods should only be used to show and collect information to the user (for whom
	 * bpm as a floating point number is the obvious representation)
	 */
	double note_types_per_minute () const { return (superclock_ticks_per_second * 60.0) / _superclocks_per_note_type; }
	double end_note_types_per_minute () const { return (superclock_ticks_per_second * 60.0) / _end_superclocks_per_note_type; }
	double quarter_notes_per_minute() const { return (superclock_ticks_per_second * 60.0 * 4.0) / (_note_type * _superclocks_per_note_type); }
	double samples_per_note_type(samplecnt_t sr) const { return superclock_to_samples (superclocks_per_note_type (), sr); }
	double samples_per_quarter_note(samplecnt_t sr) const { return superclock_to_samples (superclocks_per_quarter_note(), sr); }
	void   set_note_types_per_minute (double npm) { _superclocks_per_note_type = double_npm_to_sc (npm); }

	int note_type () const { return _note_type; }

	superclock_t superclocks_per_note_type () const {
		return _superclocks_per_note_type;
	}
	superclock_t end_superclocks_per_note_type () const {
		return _end_superclocks_per_note_type;
	}
	superclock_t superclocks_per_note_type (int note_type) const {
		return (_superclocks_per_note_type * _note_type) / note_type;
	}
	superclock_t superclocks_per_quarter_note () const {
		return superclocks_per_note_type (4);
	}
	superclock_t superclocks_per_ppqn () const {
		return superclocks_per_quarter_note() / ticks_per_beat;
	}

	bool active () const { return _active; }
	void set_active (bool yn) { _active = yn; }

	bool locked_to_meter ()  const { return _locked_to_meter; }
	void set_locked_to_meter (bool yn) { _locked_to_meter = yn; }

	bool clamped() const { return _clamped; }
	bool set_clamped (bool yn);

	Type type() const { return _type; }

	bool ramped () const { return _type != Constant; }
	bool set_ramped (bool yn);

	XMLNode& get_state () const;

  protected:
	superclock_t _superclocks_per_note_type;
	superclock_t _end_superclocks_per_note_type;
	int8_t       _note_type;
	bool         _active;
	bool         _locked_to_meter; /* XXX name has unclear meaning with nutempo */
	bool         _clamped;
	Type         _type;

	static inline double       sc_to_double_npm (superclock_t sc) { return (superclock_ticks_per_second * 60.0) / sc; }
	static inline superclock_t double_npm_to_sc (double npm) { return llrint ((superclock_ticks_per_second / npm) * 60.0); }
};

/** Meter, or time signature (subdivisions per bar, and which note type is a single subdivision). */
class LIBTEMPORAL_API Meter {
  public:

	static std::string xml_node_name;

	Meter (XMLNode const &);
	Meter (int8_t dpb, int8_t nv) : _note_value (nv), _divisions_per_bar (dpb) {}

	int divisions_per_bar () const { return _divisions_per_bar; }
	int note_value() const { return _note_value; }

	inline bool operator==(const Meter& other) { return _divisions_per_bar == other.divisions_per_bar() && _note_value == other.note_value(); }
	inline bool operator!=(const Meter& other) { return _divisions_per_bar != other.divisions_per_bar() || _note_value != other.note_value(); }

	Meter& operator=(Meter const & other) {
		if (&other != this) {
			_divisions_per_bar = other._divisions_per_bar;
			_note_value = other._note_value;
		}
		return *this;
	}

	BBT_Time   bbt_add (BBT_Time const & bbt, BBT_Offset const & add) const;
	BBT_Time   bbt_subtract (BBT_Time const & bbt, BBT_Offset const & sub) const;
	BBT_Offset bbt_delta (BBT_Time const & bbt, BBT_Time const & sub) const;

	BBT_Time round_to_bar (BBT_Time const &) const;

	Beats to_quarters (BBT_Offset const &) const;

	double samples_per_grid (Tempo const &, samplecnt_t sr) const;
	double samples_per_bar (Tempo const &, samplecnt_t sr) const;

	XMLNode& get_state () const;

  protected:
	/** The type of "note" that a division represents.  For example, 4 is
	    a quarter (crotchet) note, 8 is an eighth (quaver) note, etc.
	*/
	int8_t _note_value;
	/* how many of '_note_value' make up a bar or measure */
	int8_t _divisions_per_bar;
};

/** Helper class to keep track of the Meter *AND* Tempo in effect
    at a given point in time.
*/
class LIBTEMPORAL_API TempoMetric : public Tempo, public Meter {
  public:
	TempoMetric (Tempo const & t, Meter const & m, bool ramp)
		: Tempo (t), Meter (m), _c_per_quarter (0.0), _c_per_superclock (0.0) {}
	~TempoMetric () {}

	double c_per_superclock () const { return _c_per_superclock; }
	double c_per_quarter () const { return _c_per_quarter; }

	void compute_c_superclock (samplecnt_t sr, superclock_t end_superclocks_per_note_type, superclock_t duration);
	void compute_c_quarters (samplecnt_t sr, superclock_t end_superclocks_per_note_type, Beats const & duration);

	superclock_t superclocks_per_bar (samplecnt_t sr) const;
	superclock_t superclocks_per_grid (samplecnt_t sr) const;

	superclock_t superclock_at_qn (Beats const & qn) const;
	superclock_t superclock_per_note_type_at_superclock (superclock_t) const;

  private:
	double _c_per_quarter;
	double _c_per_superclock;
};

/** Tempo Map - mapping of timecode to musical time.
 * convert audio-samples, sample-rate to Bar/Beat/Tick, Meter/Tempo
 */

/* TempoMap concepts

   we have several different ways of talking about time:

   * PULSE : whole notes, just because. These are linearly related to any other
             note type, so if you know a number of pulses (whole notes), you
             know the corresponding number of any other note type (e.g. quarter
             notes).

   * QUARTER NOTES : just what the name says. A lot of MIDI software and
                     concepts assume that a "beat" is a quarter-note.

   * BEAT : a fraction of a PULSE. Defined by the meter in effect, so requires
            meter (time signature) information to convert to/from PULSE or QUARTER NOTES.
            In a 5/8 time, a BEAT is 1/8th note. In a 4/4 time, a beat is quarter note.
            This means that measuring time in BEATS is potentially non-linear (if
            the time signature changes, there will be a different number of BEATS
            corresponding to a given time in any other unit).

   * SUPERCLOCK : a very high resolution clock whose frequency
                  has as factors all common sample rates and all common note
                  type divisors. Related to MINUTES or SAMPLES only when a
                  sample rate is known. Related to PULSE or QUARTER NOTES only
                  when a tempo is known.

   * MINUTES : wallclock time measurement. related to SAMPLES or SUPERCLOCK
               only when a sample rate is known.


   * SAMPLES : audio time measurement. Related to MINUTES or SUPERCLOCK only
               when a sample rate is known

   * BBT : bars|beats|ticks ... linearly related to BEATS but with the added
           semantics of bars ("measures") added, in which beats are broken up
           into groups of bars ("measures"). Requires meter (time signature)
           information to compute to/from a given BEATS value. Contains no
           additional time information compared to BEATS, but does have
           additional semantic information.

  Nick sez: not every note onset is on a tick
  Paul wonders: if it's 8 samples off, does it matter?
  Nick sez: it should not phase with existing audio

 */

class LIBTEMPORAL_API TempoMapPoint
{
  public:
	enum Flag {
		ExplicitTempo =    0x1,
		ExplicitMeter =    0x2,
		ExplicitPosition = 0x4,
	};

	TempoMapPoint (XMLNode const &);
	TempoMapPoint (TempoMap* map, Flag f, Tempo const& t, Meter const& m, superclock_t sc, Beats const & q, BBT_Time const & bbt, bool ramp = false)
		: _flags (f), _explicit (t, m, ramp), _sclock (sc), _quarters (q), _bbt (bbt), _dirty (true), _floating (false), _map (map) {}
	TempoMapPoint (TempoMapPoint const & tmp, superclock_t sc, Beats const & q, BBT_Time const & bbt)
		: _flags (Flag (0)), _reference (&tmp), _sclock (sc), _quarters (q), _bbt (bbt), _dirty (true), _floating (false), _map (tmp.map()) {}
	~TempoMapPoint () {}

	TempoMap* map() const { return _map; }
	void set_map (TempoMap* m);


	/* called by a GUI that is manipulating the position of this point */
	void start_float ();
	void end_float ();
	bool floating() const { return _floating; }

	Flag flags() const             { return _flags; }
	void set_flags (Flag f);

	bool is_explicit_tempo ()    const { return _flags & ExplicitTempo; }
	bool is_explicit_meter ()    const { return _flags & ExplicitMeter; }
	bool is_explicit_position () const { return _flags & ExplicitPosition; }
	bool is_explicit() const           { return _flags & (ExplicitMeter|ExplicitTempo|ExplicitPosition); }
	bool is_implicit() const           { return _flags == Flag (0); }

	void make_explicit (Flag f) {
		if (!(_flags & f)) {
			_flags = Flag (_flags|f);
			/* since _metric and _reference are part of an anonymous union,
			   avoid possible compiler glitches by copying to a stack
			   variable first, then assign.
			*/
			TempoMetric tm (metric());
			_explicit = tm;
			_dirty = true;
		}
	}

	superclock_t superclocks_per_note_type (int8_t note_type) const {
		if (is_explicit()) {
			return _explicit.superclocks_per_note_type (note_type);
		}
		return _reference->superclocks_per_note_type (note_type);
	}

	struct BadTempoMetricLookup : public std::exception {
		virtual const char* what() const throw() { return "cannot obtain non-const Metric from implicit map point"; }
	};

	bool                dirty()  const { return _dirty; }

	superclock_t        sclock() const      { return _sclock; }

	timepos_t           time() const;
	samplepos_t         sample() const;
	Beats const &       quarters() const  { return _quarters; }
	BBT_Time  const &   bbt() const       { return _bbt; }

	bool                ramped() const      { return metric().ramped(); }

	TempoMetric const & metric() const { return is_explicit() ? _explicit : _reference->metric(); }
	Tempo               tempo() const  { return metric(); }
	Tempo               meter() const  { return metric(); }

	void compute_c_superclock (samplecnt_t sr, superclock_t end_superclocks_per_note_type, superclock_t duration) { if (is_explicit()) { _explicit.compute_c_superclock (sr, end_superclocks_per_note_type, duration); } }
	void compute_c_quarters (samplecnt_t sr, superclock_t end_superclocks_per_note_type, Beats const & duration) { if (is_explicit()) { _explicit.compute_c_quarters (sr, end_superclocks_per_note_type, duration); } }

	/* None of these properties can be set for an Implicit point, because
	 * they are determined by the TempoMapPoint pointed to by _reference.
	 */

	void set_sclock (superclock_t  sc) { if (is_explicit()) { _sclock = sc; _dirty = true; } }
	void set_quarters (Beats const & q) { if (is_explicit()) { _quarters = q; _dirty = true;  } }
	void set_bbt (BBT_Time const & bbt) {  if (is_explicit()) { _bbt = bbt; _dirty = true;  } }
	void set_dirty (bool yn);
	void make_implicit (TempoMapPoint & tmp) { _flags = Flag (0); _reference = &tmp; }

	Beats quarters_at (superclock_t sc) const;
	Beats quarters_at (BBT_Time const &) const;

	BBT_Time bbt_at (Beats const &) const;
	BBT_Time bbt_at (superclock_t) const;

	XMLNode& get_state() const;
	int set_state (XMLNode const&, int version);

	struct SuperClockComparator {
		bool operator() (TempoMapPoint const & a, TempoMapPoint const & b) const { return a.sclock() < b.sclock(); }
	};

	struct QuarterComparator {
		bool operator() (TempoMapPoint const & a, TempoMapPoint const & b) const { return a.quarters() < b.quarters(); }
	};

	struct BBTComparator {
		bool operator() (TempoMapPoint const & a, TempoMapPoint const & b) const { return a.bbt() < b.bbt(); }
	};

	superclock_t  walk_to_superclock (superclock_t start, Beats const & distance) const;
	Beats walk_to_quarters (superclock_t start, superclock_t distance) const;

	TempoMetric & nonconst_metric() const { return is_explicit() ? *(const_cast<TempoMetric*>(&_explicit)) : *(const_cast<TempoMetric*>(&_reference->metric())); }

  protected:
	friend class TempoMap;
	void map_reset_set_sclock_for_sr_change (superclock_t sc) { _sclock = sc; }

  private:
	Flag                  _flags;
	union {
		TempoMapPoint const * _reference;
		TempoMetric           _explicit;
	};
	superclock_t          _sclock;
	Beats                 _quarters;
	BBT_Time              _bbt;
	bool                  _dirty;
	bool                  _floating;
	TempoMap*             _map;
};

typedef std::list<TempoMapPoint> TempoMapPoints;

class LIBTEMPORAL_API TempoMap : public PBD::StatefulDestructible
{
   public:
	TempoMap (Tempo const & initial_tempo, Meter const & initial_meter, samplecnt_t sr);
	~TempoMap();

	void set_dirty (bool yn);

	void set_sample_rate (samplecnt_t sr);
	samplecnt_t sample_rate() const { return _sample_rate; }

	void insert_time (timepos_t const & pos, timecnt_t const & duration);
	bool remove_time (timepos_t const & pos, timecnt_t const & duration);

	bool set_tempo_and_meter (Tempo const &, Meter const &, samplepos_t, bool ramp, bool flexible);

	void change_tempo (TempoMapPoint&, Tempo const &);

	/* these will return null if the tempo could not be set at the * requested time
	 */
	TempoMapPoint* set_tempo (Tempo const &, BBT_Time const &, bool ramp = false);
	TempoMapPoint* set_tempo (Tempo const &, samplepos_t, bool ramp = false);
	TempoMapPoint* set_tempo (Tempo const &, Beats const &, bool ramp = false);
	TempoMapPoint* set_tempo (Tempo const &, timepos_t const &, bool ramp = false);

	void remove_tempo_at (TempoMapPoint const &);

	bool set_meter (Meter const &, BBT_Time const &);
	bool set_meter (Meter const &, samplepos_t);
	bool set_meter (Meter const &, timepos_t const &);

	void remove_meter_at (TempoMapPoint const &);

	void remove_explicit_point (samplepos_t);

	/* these are a convenience method that just wrap some odd semantics */
	bool move_to (timepos_t const & current, timepos_t const & destination, bool push = false);
	bool move_to (TempoMapPoint & point, timepos_t const & destination, bool push = false);

	bool can_remove (Tempo const &) const;
	bool can_remove (Meter const &) const;

	bool is_initial (Tempo const &) const;
	bool is_initial (Meter const &) const;

	uint32_t n_meters() const;
	uint32_t n_tempos() const;

	Tempo const * next_tempo (Tempo const &) const;
	Meter const * next_meter (Meter const &) const;

	Meter const & meter_at (samplepos_t sc) const;
	Meter const & meter_at (Beats const & b) const;
	Meter const & meter_at (BBT_Time const & bbt) const;
	Meter const & meter_at (timepos_t const &) const;

	Tempo const & tempo_at (samplepos_t sc) const;
	Tempo const & tempo_at (Beats const &b) const;
	Tempo const & tempo_at (BBT_Time const & bbt) const;
	Tempo const & tempo_at (timepos_t const & t) const;

	TempoMapPoint const * previous_tempo (TempoMapPoint const &) const;

	/* convenience function */
	BBT_Time round_to_bar (BBT_Time const & bbt) const {
		return const_point_at (bbt).metric().round_to_bar (bbt);
	}

	BBT_Time bbt_at (samplepos_t sc) const;
	BBT_Time bbt_at (Beats const &) const;
	BBT_Time bbt_at (timepos_t const &) const;

	Beats quarter_note_at (samplepos_t sc) const;
	Beats quarter_note_at (BBT_Time const &) const;
	Beats quarter_note_at (timepos_t const &) const;

	samplepos_t sample_at (Beats const &) const;
	samplepos_t sample_at (BBT_Time const &) const;

	int update_music_times (int gen, samplepos_t, Beats & b, BBT_Time & bbt, bool force);
	int update_samples_and_beat_times (int gen, BBT_Time const & bbt, samplepos_t & pos, Beats & b, bool force);
	int update_samples_and_bbt_times (int gen, Beats const & b, samplepos_t & pos, BBT_Time & bbt, bool force);

	/* ways to walk along the tempo map, measure distance between points,
	 * etc.
	 */

	Beats sample_delta_as_quarters (samplepos_t start, samplepos_t distance) const;
	Beats samplewalk_to_quarters (samplepos_t pos, samplecnt_t distance) const;
	Beats samplewalk_to_quarters (Beats const & pos, samplecnt_t distance) const;
	samplepos_t sample_plus_quarters_as_samples (samplepos_t start, Beats const & distance) const;
	samplepos_t sample_quarters_delta_as_samples (samplepos_t start, Beats const & distance) const;
	samplepos_t samplepos_plus_bbt (samplepos_t pos, BBT_Time op) const;

	samplecnt_t bbt_duration_at (samplepos_t pos, const BBT_Time& bbt, int dir) const;
	Beats bbtwalk_to_quarters (Beats const & start, BBT_Offset const & distance) const;

	samplecnt_t samples_per_quarter_note_at (samplepos_t) const;

	Temporal::timecnt_t full_duration_at (Temporal::timepos_t const &, Temporal::timecnt_t const & duration, Temporal::LockStyle domain) const;

	BBT_Time bbt_walk (BBT_Time const &, BBT_Offset const &) const;

	TempoMapPoint const & const_point_at (samplepos_t s) const { return *const_iterator_at (samples_to_superclock (s, _sample_rate)); }
	TempoMapPoint const & const_point_at (Beats const & b) const { return *const_iterator_at (b); }
	TempoMapPoint const & const_point_at (BBT_Time const & bbt) const { return *const_iterator_at (bbt); }

	TempoMapPoint const & const_point_after (samplepos_t sc) const;
	TempoMapPoint const & const_point_after (Beats const & b) const;
	TempoMapPoint const & const_point_after (BBT_Time const & bbt) const;

	LockStyle time_domain() const { return _time_domain; }
	void set_time_domain (LockStyle td);

	/* If resolution == Beats() (i.e. zero), then the grid that is
	   returned will contain a mixture of implicit and explicit points,
	   and will only be valid as long as this map remains unchanged
	   (because the implicit points may reference explicit points in the
	   map.

	   If resolution != Beats() (i.e. non-zero), then the in-out @param
	   grid will contain only explicit points that do not reference this
	   map in anyway.
	*/

	void get_grid (TempoMapPoints& points, samplepos_t start, samplepos_t end, Beats const & resolution);
	void get_bar_grid (TempoMapPoints& points, samplepos_t start, samplepos_t end, int32_t bar_gap);

	/* returns all points with ExplicitMeter and/or ExplicitTempo */
	void get_points (TempoMapPoints& points) const;
	void get_tempos (TempoMapPoints& points) const;
	void get_meters (TempoMapPoints& points) const;

	template<class T> void apply_with_points (T& obj, void (T::*method)(TempoMapPoints &)) {
		Glib::Threads::RWLock::ReaderLock lm (_lock);
		(obj.*method)(_points);
	}

	struct EmptyTempoMapException : public std::exception {
		virtual const char* what() const throw() { return "TempoMap is empty"; }
	};

	void dump (std::ostream&);
	void rebuild (superclock_t limit);
	void full_rebuild ();

	PBD::Signal2<void,samplepos_t,samplepos_t> Changed;

	XMLNode& get_state();
	int set_state (XMLNode const&, int version);

   private:
	TempoMapPoints                _points;
	samplecnt_t                   _sample_rate;
	mutable Glib::Threads::RWLock _lock;
	bool                          _dirty;
	int                           _generation;
	LockStyle                     _time_domain;

	/* these return an iterator that refers to the TempoMapPoint at or most immediately preceding the given position.
	 *
	 * Conceptually, these could be const methods, but C++ prevents them returning a non-const iterator in that case.
	 *
	 * Note that they cannot return an invalid iterator (e.g. _points.end()) because:
	 *
	 *    - if the map is empty, an exception is thrown
	 *    - if the given time is before the first map entry, _points.begin() is returned
	 *    - if the given time is after the last map entry, the equivalent of _points.rbegin() is returned
	 *    - if the given time is within the map entries, a valid iterator will be returned
	 *
	 * The caller MUST hold a read or write lock on the map.
	 */

	TempoMapPoints::iterator iterator_at (superclock_t sc);
	TempoMapPoints::iterator iterator_at (Beats const &);
	TempoMapPoints::iterator iterator_at (BBT_Time const &);

	TempoMapPoints::const_iterator const_iterator_at (superclock_t sc) const { return const_cast<TempoMap*>(this)->iterator_at (sc); }
	TempoMapPoints::const_iterator const_iterator_at (Beats const & b) const { return const_cast<TempoMap*>(this)->iterator_at (b); }
	TempoMapPoints::const_iterator const_iterator_at (BBT_Time const & bbt) const { return const_cast<TempoMap*>(this)->iterator_at (bbt); }

	/* Returns the TempoMapPoint at or most immediately preceding the given time. If the given time is
	 * before the first map entry, then the first map entry will be returned, which underlies the semantics
	 * that the first map entry's values propagate backwards in time if not at absolute zero.
	 *
	 * As for iterator_at(), define both const+const and non-const variants, because C++ won't let us return a non-const iterator
	   from a const method (which is a bit silly, but presumably aids compiler reasoning).
	*/

	TempoMapPoint & point_at (superclock_t sc) { return *iterator_at (sc); }
	TempoMapPoint & point_at (Beats const & b) { return *iterator_at (b); }
	TempoMapPoint & point_at (BBT_Time const & bbt) { return *iterator_at (bbt); }

	Meter const & meter_at_locked (superclock_t sc) const { return const_point_at (sc).metric(); }
	Meter const & meter_at_locked (Beats const & b) const { return const_point_at (b).metric(); }
	Meter const & meter_at_locked (BBT_Time const & bbt) const { return const_point_at (bbt).metric(); }
	Tempo const & tempo_at_locked (superclock_t sc) const { return const_point_at (sc).metric(); }
	Tempo const & tempo_at_locked (Beats const &b) const { return const_point_at (b).metric(); }
	Tempo const & tempo_at_locked (BBT_Time const & bbt) const { return const_point_at (bbt).metric(); }
	BBT_Time      bbt_at_locked (superclock_t sc) const;
	BBT_Time      bbt_at_locked (Beats const &) const;
	samplepos_t   sample_at_locked (Beats const &) const;
	samplepos_t   sample_at_locked (BBT_Time const &) const;

	void maybe_rebuild();
	void rebuild_locked (superclock_t limit);
	void dump_locked (std::ostream&);

	bool move_to (TempoMapPoints::iterator & current, timepos_t const & destination, bool push);
};

} /* end of namespace Temporal */

#ifdef COMPILER_MSVC
#pragma warning(disable:4101)
#endif

namespace PBD {
DEFINE_ENUM_CONVERT(Temporal::Tempo::Type);
DEFINE_ENUM_CONVERT(Temporal::LockStyle);
}


namespace std {
std::ostream& operator<<(std::ostream&, Temporal::TempoMapPoint const &);
std::ostream& operator<<(std::ostream&, Temporal::Tempo const &);
std::ostream& operator<<(std::ostream&, Temporal::Meter const &);
}

#endif /* __temporal_tempo_h__ */