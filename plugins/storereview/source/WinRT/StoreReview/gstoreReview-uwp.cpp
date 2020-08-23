#include "gstoreReview.h"
#include <windows.services.store.h>
#include "giderosapi.h"
#include <ppltasks.h>
using namespace Concurrency;
using namespace Windows::Services::Store;

bool gstorereview_requestReview()
{
	StoreContext ^ctx = StoreContext::GetDefault();
	if (ctx != nullptr)
		gdr_dispatchUi([&]()
		{
			create_task(ctx->RequestRateAndReviewAppAsync()).then([=](task<StoreRateAndReviewResult^> previousTask)
			{
			});
		}, false);

    return true;
}
