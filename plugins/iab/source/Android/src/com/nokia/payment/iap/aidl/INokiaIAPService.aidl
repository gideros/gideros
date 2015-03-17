/*
Copyright (C) 2013 Nokia Corporation. All rights reserved.
*
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
*
http://www.apache.org/licenses/LICENSE-2.0
*
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

package com.nokia.payment.iap.aidl;

import android.os.Bundle;

/**
 * All calls will give a response code with the following possible values<br/><br/>
 * RESULT_OK = 0 - success<br/>
 * RESULT_USER_CANCELED = 1 - user pressed back or canceled a dialog<br/>
 * RESULT_BILLING_UNAVAILABLE = 3 - this billing API version is not supported for the type requested or billing is otherwise impossible<br/>
 * RESULT_ITEM_UNAVAILABLE = 4 - requested ProductID is not available for purchase<br/>
 * RESULT_DEVELOPER_ERROR = 5 - invalid arguments provided to the API<br/>
 * RESULT_ERROR = 6 - Fatal error during the API action<br/>
 * RESULT_ITEM_ALREADY_OWNED = 7 - Failure to purchase since item is already owned<br/>
 * RESULT_ITEM_NOT_OWNED = 8 - Failure to consume since item is not owned<br/>
 * RESULT_NO_SIM = 9 - Billing is not available because there is no SIM card inserted<br/>
 *<br/>
 * Only supported itemtype, other values result in RESULT_DEVELOPER_ERROR<br/>
 * ITEM_TYPE_INAPP = "inapp";<br/>
 */
interface INokiaIAPService {
    /**
     * Checks support for the requested billing API version, package and in-app type.
     * @param apiVersion the billing version which the app is using
     * @param packageName the package name of the calling app
       @param type must always be "inapp"     
     * @return RESULT_OK(0) on success, corresponding result code on failures
     */
    int isBillingSupported(int apiVersion, String packageName, String type );

    /**
     * Provides details of a list of products<br/>
     * Given a list of Productids of a valid type in the productBundle, this returns a bundle
     * with a list JSON strings containing the productId, price, title and description.
     * This API can be called with a maximum of 20 Productids. 
     * @param apiVersion billing API version that the Third-party is using
     * @param packageName the package name of the calling app
     * @param type must always be "inapp"     
     * @param productBundle bundle containing a StringArrayList of Productids with key "ITEM_ID_LIST", when setProductMappings has been called this parameter can be null
     * @return Bundle containing the following key-value pairs<br/>
     *         "RESPONSE_CODE" with int value, RESULT_OK(0) if success, other response codes on
     *              failure as listed above.<br/>
     *         "DETAILS_LIST" with a StringArrayList containing purchase information
     *              in JSON format similar to:<br/>
     <br/>
     *              '{ "productId" : "1264321", "isvalid", true, "title" : "Product title",<br/> 
     *				    "shortdescription" : "Short description of the product", <br/>
     *                  "description" : "Longer description of the product", "priceValue" : "3.00",<br/>
     *                  "price" : "$3.00", "currency", "USD",<br/>
     *                  "purchaseToken" : "ZXlKMlpYSWlPaUl4TGpBaUxDSjBlRzVKWkNJNklrNVFRVmxmVkVWVFZGOVVXRTVmTVRFeE1TSXNJbkJ5YjJSSlpDSTZJakV3TWpNMk1qUWlmUT09",<br/>
     *                  "taxesincluded": true, "restorable" : true, "type" : "inapp" }'<br/><br/>
     *  or if requested productId is not valid then<br/>
     *              ''{ "productId" : "invalidproductid", "isvalid", false }'<br/>
     */
    Bundle getProductDetails(int apiVersion, String packageName, String type, in Bundle productBundle);

    /**
     * Returns a pending intent to launch the purchase flow for an in-app item by providing a ProductID,
     * the type, a unique purchase token and an optional developer payload.
     * @param apiVersion billing API version that the app is using
     * @param packageName package name of the calling app
     * @param productId the ProductID of the in-app item as published in the developer console or as mapped through setProductMappings
     * @param type must always be "inapp"
     * @param developerPayload optional argument to be sent back with the purchase information
     * @return Bundle containing the following key-value pairs<br/><br/>
     *         "RESPONSE_CODE" with int value, RESULT_OK(0) if success, other response codes on
     *              failure as listed above.<br/>
     *         "BUY_INTENT" - PendingIntent to start the purchase flow
     *
     * The Pending intent should be launched with startIntentSenderForResult. When purchase flow
     * has completed, the onActivityResult() will give a resultCode of OK or CANCELED.<br/><br/>
     * If the purchase is successful, the result data will contain the following key-value pairs<br/><br/>
     *         "RESPONSE_CODE" with int value, RESULT_OK(0) if success, other response codes on
     *              failure as listed above.<br/>
     *         "INAPP_PURCHASE_DATA" - String in JSON format similar to<br/><br/>
     *              '{"orderId":"X393XDAFFDAFAD",<br/>
     *                "packageName":"com.your.app",<br/>
     *                "productId":"1264321",<br/>
     *                "purchaseToken" : "ZXlKMlpYSWlPaUl4TGpBaUxDSjBlRzVKWkNJNklrNVFRVmxmVkVWVFZGOVVXRTVmTVRFeE1TSXNJbkJ5YjJSSlpDSTZJakV3TWpNMk1qUWlmUT09",<br/>
     *                "developerPayload":"" }'<br/><br/>
     *         "INAPP_DATA_SIGNATURE" - currently empty string<br/>
     */
    Bundle getBuyIntent(int apiVersion, String packageName, String productID, String type,
        String developerPayload);

    /**
     * Returns the current products associated with current imei
     * @param apiVersion billing API version that the app is using
     * @param packageName package name of the calling app
     * @param type must always be "inapp"
     * @param productBundle bundle containing a StringArrayList of Productids with key "ITEM_ID_LIST", when setProductMappings has been called this parameter can be null
     * @param continuationToken - currently ignored    
     * @return Bundle containing the following key-value pairs<br/>
     *         "RESPONSE_CODE" with int value, RESULT_OK(0) if success, other response codes on
     *              failure as listed above.<br/>
     *         "INAPP_PURCHASE_ITEM_LIST" - StringArrayList containing the list of owned products<br/>
     *         "INAPP_PURCHASE_DATA_LIST" - StringArrayList containing for each owned product json string
     *             similar to following (please note that delveloperPayload field is currently empty):<br/><br/>
     *                { "productId":"1264321",<br/>
     *                "purchaseToken" : "ZXlKMlpYSWlPaUl4TGpBaUxDSjBlRzVKWkNJNklrNVFRVmxmVkVWVFZGOVVXRTVmTVRFeE1TSXNJbkJ5YjJSSlpDSTZJakV3TWpNMk1qUWlmUT09",<br/>
     *                "developerPayload":"" }<br/>
     */
    Bundle getPurchases(int apiVersion, String packageName, String type, in Bundle productBundle, String continuationToken );

    /**
     * Consume the last purchase of the given product. This will result in this item being removed
     * from all subsequent responses to getPurchases() and allow re-purchase of this item.
     * @param apiVersion billing API version that the app is using
     * @param packageName package name of the calling app
     * @param productId productId of purchase to be consumed, this argument can be empty or null
     * @param purchaseToken token in the purchase information JSON that identifies the purchase
     *        to be consumed
     * @return 0 if consumption succeeded. Appropriate error values for failures.
     */
    int consumePurchase(int apiVersion, String packageName, String productId, String purchaseToken);
    
    /**
     * Set mapping between nokia productid-s and application internal productid-s
     * @param apiVersion billing API version that the app is using
     * @param packageName package name of the calling app
     * @param mappingsBundle - bundle containing mapping as key, value pairs where key is Nokia productid and value is calling applications internal id 
    */
    
    int setProductMappings( int apiVersion, String packageName, in Bundle mappingsBundle );
}
