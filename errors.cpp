#include "errors.hpp"


static bool HCF = false;


void SetHaltAndCatchFire()
{ 
	HCF = true;
}


bool GetHaltAndCatchFire()
{
	return HCF;
}
