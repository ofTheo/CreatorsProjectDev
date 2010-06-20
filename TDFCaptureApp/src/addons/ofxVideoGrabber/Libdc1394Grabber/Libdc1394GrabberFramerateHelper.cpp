#include "Libdc1394GrabberFramerateHelper.h"


/*
DC1394_FRAMERATE_1_875= 32,
  DC1394_FRAMERATE_3_75,
  DC1394_FRAMERATE_7_5,
  DC1394_FRAMERATE_15,
  DC1394_FRAMERATE_30,
  DC1394_FRAMERATE_60,
  DC1394_FRAMERATE_120,
  DC1394_FRAMERATE_240
*/


dc1394framerate_t Libdc1394GrabberFramerateHelper::numToDcLibFramerate( int rateNum  )
{

	switch (rateNum )
	{

	case 1:
		return DC1394_FRAMERATE_1_875;
		break;

	case 2:
		return DC1394_FRAMERATE_1_875;
		break;

	case 3:
		return DC1394_FRAMERATE_3_75;
		break;

	case 4:
		return DC1394_FRAMERATE_3_75;
		break;


	case 7:
		return DC1394_FRAMERATE_7_5;
		break;

	case 8:
		return DC1394_FRAMERATE_7_5;
		break;


	case 15:
		return DC1394_FRAMERATE_15;
		break;

	case 24:
		return DC1394_FRAMERATE_30;
		break;

	case 25:
		return DC1394_FRAMERATE_30;
		break;

	case 30:
		return DC1394_FRAMERATE_30;
		break;

	case 60:
		return DC1394_FRAMERATE_60;
		break;

	case 120:
		return DC1394_FRAMERATE_120;
		break;

	case 240:
		return DC1394_FRAMERATE_240;
		break;

	default:
		return DC1394_FRAMERATE_15;
		break;
	}

}


const char* Libdc1394GrabberFramerateHelper::DcLibFramerateToString( dc1394framerate_t _framerate )
{
	switch ( _framerate )
	{

		case DC1394_FRAMERATE_1_875:
			return "DC1394_FRAMERATE_1_875";
			break;

		case DC1394_FRAMERATE_3_75:
			return "DC1394_FRAMERATE_3_75";
			break;

		case DC1394_FRAMERATE_7_5:
			return "DC1394_FRAMERATE_7_5";
			break;
		case DC1394_FRAMERATE_15:
			return "DC1394_FRAMERATE_15";
			break;

		case DC1394_FRAMERATE_30:
			return "DC1394_FRAMERATE_30";
			break;

		case DC1394_FRAMERATE_60:
			return "DC1394_FRAMERATE_60";
			break;

		case DC1394_FRAMERATE_120:
			return "DC1394_FRAMERATE_120";
			break;

		case DC1394_FRAMERATE_240:
			return "DC1394_FRAMERATE_240";
			break;

		default:
			return "** Libdc1394GrabberFramerateHelper::DcLibFramerateToString, unknown frame rate ";
			break;

	}

}
