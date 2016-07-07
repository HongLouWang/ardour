#include "mode.h"

MusicalMode::MusicalMode (MusicalMode::Type t)
{
	fill (*this, t);
}

MusicalMode::~MusicalMode()
{
}

void
MusicalMode::fill (MusicalMode& m, MusicalMode::Type t)
{
	m.steps.clear ();

	/* scales/modes as distances from root, expressed
	   in fractional whole tones.
	*/

	switch (t) {
	case Random:
		m.steps.push_back (0.0); // sekrit code for "random"
		break;
	case Dorian:
		m.steps.push_back (1.0);
		m.steps.push_back (1.5);
		m.steps.push_back (2.0);
		m.steps.push_back (3.0);
		m.steps.push_back (4.0);
		m.steps.push_back (4.5);
		break;
	case Ionian:
		m.steps.push_back (1.0);
		m.steps.push_back (2.0);
		m.steps.push_back (2.5);
		m.steps.push_back (3.5);
		m.steps.push_back (4.5);
		m.steps.push_back (5.5);
		break;
	case Phrygian:
		m.steps.push_back (0.5);
		m.steps.push_back (1.5);
		m.steps.push_back (2.5);
		m.steps.push_back (3.5);
		m.steps.push_back (4.0);
		m.steps.push_back (5.0);
		break;
	case Lydian:
		m.steps.push_back (1.0);
		m.steps.push_back (2.0);
		m.steps.push_back (3.0);
		m.steps.push_back (3.5);
		m.steps.push_back (4.5);
		m.steps.push_back (5.5);
		break;
	case Mixolydian:
		m.steps.push_back (1.0);
		m.steps.push_back (2.0);
		m.steps.push_back (2.5);
		m.steps.push_back (3.5);
		m.steps.push_back (4.5);
		m.steps.push_back (5.0);
		break;
	case Aeolian:
		m.steps.push_back (1.0);
		m.steps.push_back (1.5);
		m.steps.push_back (2.5);
		m.steps.push_back (3.5);
		m.steps.push_back (4.0);
		m.steps.push_back (5.0);
		break;
	case Locrian:
		m.steps.push_back (0.5);
		m.steps.push_back (1.5);
		m.steps.push_back (2.0);
		m.steps.push_back (3.0);
		m.steps.push_back (4.0);
		m.steps.push_back (5.0);
		break;
	case PentatonicMajor:
		m.steps.push_back (1.0);
		m.steps.push_back (2.0);
		m.steps.push_back (2.5);
		m.steps.push_back (3.5);
		break;
	case PentatonicMinor:
		m.steps.push_back (1.5);
		m.steps.push_back (2.5);
		m.steps.push_back (3.5);
		m.steps.push_back (5.0);
		break;
	case  MajorChord:
		m.steps.push_back (2.0);
		m.steps.push_back (3.5);
		break;
	case  MinorChord:
		m.steps.push_back (1.5);
		m.steps.push_back (3.5);
		break;
	case  Min7:
		m.steps.push_back (1.5);
		m.steps.push_back (3.5);
		m.steps.push_back (5.0);
		break;
	case  Sus4:
		m.steps.push_back (2.5);
		m.steps.push_back (3.5);
		break;
	case  Chromatic:
		m.steps.push_back (0.5);
		m.steps.push_back (1.0);
		m.steps.push_back (1.5);
		m.steps.push_back (2.0);
		m.steps.push_back (2.5);
		m.steps.push_back (3.0);
		m.steps.push_back (3.5);
		m.steps.push_back (4.0);
		m.steps.push_back (4.5);
		m.steps.push_back (5.0);
		m.steps.push_back (5.5);
		break;
	case  BluesScale:
		m.steps.push_back (1.0);
		m.steps.push_back (1.5);
		m.steps.push_back (2.5);
		m.steps.push_back (3);
		m.steps.push_back (3.5);
		m.steps.push_back (4.5);
		m.steps.push_back (5.0);
		m.steps.push_back (5.5);
		break;
	case  NeapolitanMinor:
		m.steps.push_back (0.5);
		m.steps.push_back (1.5);
		m.steps.push_back (2.5);
		m.steps.push_back (2.5);
		m.steps.push_back (4.0);
		m.steps.push_back (5.5);
		break;
	case  NeapolitanMajor:
		m.steps.push_back (0.5);
		m.steps.push_back (1.5);
		m.steps.push_back (2.5);
		m.steps.push_back (3.5);
		m.steps.push_back (4.5);
		m.steps.push_back (5.5);
		break;
	case  Oriental:
		m.steps.push_back (0.5);
		m.steps.push_back (2.0);
		m.steps.push_back (2.5);
		m.steps.push_back (3.0);
		m.steps.push_back (4.5);
		m.steps.push_back (5.0);
		break;
	case  DoubleHarmonic:
		m.steps.push_back (0.5);
		m.steps.push_back (2.0);
		m.steps.push_back (2.5);
		m.steps.push_back (3.5);
		m.steps.push_back (4.0);
		m.steps.push_back (5.5);
		break;
	case  Enigmatic:
		m.steps.push_back (0.5);
		m.steps.push_back (2.0);
		m.steps.push_back (3.0);
		m.steps.push_back (4.0);
		m.steps.push_back (5.0);
		m.steps.push_back (5.5);
		break;
	case  Hirajoshi:
		m.steps.push_back (1.0);
		m.steps.push_back (1.5);
		m.steps.push_back (3.5);
		m.steps.push_back (4.0);
		break;
	case  HungarianMinor:
		m.steps.push_back (1.0);
		m.steps.push_back (1.5);
		m.steps.push_back (3.0);
		m.steps.push_back (3.5);
		m.steps.push_back (4.0);
		m.steps.push_back (5.5);
		break;
	case  HungarianMajor:
		m.steps.push_back (1.0);
		m.steps.push_back (2.0);
		m.steps.push_back (3.0);
		m.steps.push_back (3.5);
		m.steps.push_back (4.0);
		m.steps.push_back (5.0);
		break;
	case  Kumoi:
		m.steps.push_back (0.5);
		m.steps.push_back (2.5);
		m.steps.push_back (3.5);
		m.steps.push_back (4.0);
		break;
	case  Iwato:
		m.steps.push_back (0.5);
		m.steps.push_back (2.5);
		m.steps.push_back (3.0);
		m.steps.push_back (5.0);
		break;
	case  Hindu:
		m.steps.push_back (1.0);
		m.steps.push_back (2.0);
		m.steps.push_back (2.5);
		m.steps.push_back (3.5);
		m.steps.push_back (4.0);
		m.steps.push_back (5.0);
		break;
	case  Spanish8Tone:
		m.steps.push_back (0.5);
		m.steps.push_back (1.5);
		m.steps.push_back (2.0);
		m.steps.push_back (2.5);
		m.steps.push_back (3.0);
		m.steps.push_back (4.0);
		m.steps.push_back (5.0);
		break;
	case  Pelog:
		m.steps.push_back (0.5);
		m.steps.push_back (1.5);
		m.steps.push_back (3.5);
		m.steps.push_back (5.0);
		break;
	case  HungarianGypsy:
		m.steps.push_back (1.0);
		m.steps.push_back (1.5);
		m.steps.push_back (3.0);
		m.steps.push_back (3.5);
		m.steps.push_back (4.0);
		m.steps.push_back (5.0);
		break;
	case  Overtone:
		m.steps.push_back (1.0);
		m.steps.push_back (2.0);
		m.steps.push_back (3.0);
		m.steps.push_back (3.5);
		m.steps.push_back (4.5);
		m.steps.push_back (5.0);
		break;
	case  LeadingWholeTone:
		m.steps.push_back (1.0);
		m.steps.push_back (2.0);
		m.steps.push_back (3.0);
		m.steps.push_back (4.0);
		m.steps.push_back (5.0);
		m.steps.push_back (5.5);
		break;
	case  Arabian:
		m.steps.push_back (1.0);
		m.steps.push_back (2.0);
		m.steps.push_back (2.5);
		m.steps.push_back (3.0);
		m.steps.push_back (4.0);
		m.steps.push_back (5.0);
		break;
	case  Balinese:
		m.steps.push_back (0.5);
		m.steps.push_back (1.5);
		m.steps.push_back (3.5);
		m.steps.push_back (4.0);
		break;
	case  Gypsy:
		m.steps.push_back (0.5);
		m.steps.push_back (2.0);
		m.steps.push_back (2.5);
		m.steps.push_back (3.5);
		m.steps.push_back (4.0);
		m.steps.push_back (5.5);
		break;
	case  Mohammedan:
		m.steps.push_back (1.0);
		m.steps.push_back (1.5);
		m.steps.push_back (2.5);
		m.steps.push_back (3.5);
		m.steps.push_back (4.0);
		m.steps.push_back (5.5);
		break;
	case  Javanese:
		m.steps.push_back (0.5);
		m.steps.push_back (1.5);
		m.steps.push_back (2.5);
		m.steps.push_back (3.5);
		m.steps.push_back (4.5);
		m.steps.push_back (5.0);
		break;
	case  Persian:
		m.steps.push_back (0.5);
		m.steps.push_back (2.0);
		m.steps.push_back (2.5);
		m.steps.push_back (3.0);
		m.steps.push_back (4.0);
		m.steps.push_back (5.5);
		break;
	case  Algerian:
		m.steps.push_back (1.0);
		m.steps.push_back (1.5);
		m.steps.push_back (3.0);
		m.steps.push_back (3.5);
		m.steps.push_back (4.0);
		m.steps.push_back (5.5);
		m.steps.push_back (6.0);
		m.steps.push_back (7.0);
		m.steps.push_back (7.5);
		m.steps.push_back (8.5);
		break;
	}
}