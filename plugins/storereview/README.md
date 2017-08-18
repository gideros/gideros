# Gideros Store Review

SKStoreReviewController plugin for Gideros 

# Install
* Copy contents of source/IOS directory into your exported project's Plugins directory
* Add copied files to your Xcode project (Check create groups for added folders)

# Usage
<pre>
require("storeReview")
local hasShown = storeReview:requestReview()
print(hasShown)
</pre>
