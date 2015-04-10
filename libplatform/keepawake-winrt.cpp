using namespace Windows::System::Display;

DisplayRequest^ dispRequest;
bool requested = false;

void setKeepAwake(bool awake)
{
	if (!dispRequest){
		dispRequest = ref new DisplayRequest();
	}
	if (awake != requested){
		requested = !requested;
		if (awake)
			dispRequest->RequestActive();
		else
			dispRequest->RequestRelease();
	}
}
