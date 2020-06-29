/*
 
Android native map overlay class 
 
function Map:init(latitude, longitude, zoom, witdth, height, positionX, positionY)
Status: Tested. Constructor for the Map class, calls set

function Map:setPosition(positionX, positionY) � position the Map on the screen, relative to its parent, whether or not Map uses Sprite as a base class
Status: Tested. Implemented in Map.setPosition() 

function Map:setType(mapType) -- support "normal", "satellite", "terrain", "hybrid" (satellite with roads overlaid)
Status: Tested. Implemented in Map.setType()

function Map:setCenter(latitude, longitude) -- move the map to view the specified coordinates
Status: Tested. Implemented in map.setCenterCoordinates()

function Map:setZoom(zoomLevel) -- control the zoom level of the map, units TBD
Status: Tested. Implemented in Map.setZoom() - Uses zoom level directly. iOS version will need to map to comparable altitudes

function Map:setLocationEnabled(locationEnabled) -- at least for Android, enable the default �go to my location� button overlaid on the map
Status: Tested. Implemented in Map.setMyLocationEnabled()

function Map:onMapClickListener( event handler...)-- handle touches on the map, i.e. for selecting a location
Status: Interface change - Call mapClicked() to see if a click occurred since last call, call getMapClickLatitude(), getMapClickLongitude() to get coordinates

function Map:addMarker(marker) -- add a default style marker to the map � see below
Status: Tested. Interface change- Takes latitude, longitude, title, teturns int index of new marker

function Map:clear() -- remove all markers
Status: Tested. Implemented in Map.clear()

function Map:addEventListener(eventType, eventHandlerFunction) -- Support eventType "move", detecting when the map has been moved manually. Not strictly necessary, but useful when supporting navigation, or any scenario where you might program the app to move and/or zoom the map automatically, unless the user has manually moved it.
Status: Interface change.  Use Map.hasMoved() to find out if map has moved since last call.

function Map:getLocation() returns latitude and longitude of center of map
Status: Tested. Implemented via getCenterLatitude(), getCenterLongitude.  A Lua cover function could return both, or they can be kept separate.
 
function MapMarker:init(latitude, longitude, title) -- Sets the coordinates and name of a marker
Status: Not needed - use Map.addMarker

function MapMarker:setAlpha(alpha) � 0 to 100, sets opacity of marker from 0 (transparent) to 100 (opaque)
Status: Tested. Implemented via Map.setMarkerAlpha()

function MapMarker:setHue(hue) -- 0 to 360, angle to position on color wheel, from 0 (red) through orange and yellow to green (180) through blue and purple back to red (360.) Some other set of hue values could be used and mapped to platform specific values in the native code.
Status: Tested. Implemented via Map.setMarkerHue()

function MapMarker:addEventListener(eventType, eventHandlerFunction) -- Support clicks/taps on each marker
Status: Interface change, getClickedMarkerIndex() will return index of clicked marker, if any

function MapMarker:setId(id) -- A generic ID one could use to associate a specific marker with additional data
function MapMarker:getId(id) -- A generic ID one could use to associate a specific marker with additional data
Status: Interface change; Map.addMarker() returns index of each new marker. 

function MapMarker:getTitle() -- Returns the title specified in init(). Not strictly necessary if IDs are supported, as the calling code could keep track of the titles of all markers along with any other data specific to each marker that was added.
Status: Tested. Map.getMarkerTitle()

function MapMarker:getLocation() -- Returns latitude and longitude specified in init(). Not strictly necessary if IDs are supported, as the calling code could keep track of the coordinates of all markers along with any other data specific to each marker that was added.
Status: Tested. Interface change: Map.getMarkerLatitude(), Map.getMarkerLongitude

function Map:setMarkerCoordinates()
Status: Tested.

*/

package com.giderosmobile.android.plugins.mapplugin;

/*
 * Use the Google Maps API to add a map on top of the main activity of the app.
 */

import java.util.ArrayList;

import android.os.Handler;
import androidx.fragment.app.FragmentActivity;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentTransaction;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
import android.widget.RelativeLayout.LayoutParams;

import com.google.android.gms.maps.CameraUpdateFactory;
import com.google.android.gms.maps.GoogleMap;
import com.google.android.gms.maps.GoogleMap.OnCameraChangeListener;
import com.google.android.gms.maps.GoogleMap.OnMapClickListener;
import com.google.android.gms.maps.GoogleMap.OnMarkerClickListener;

import com.google.android.gms.maps.GoogleMapOptions;
import com.google.android.gms.maps.SupportMapFragment;
import com.google.android.gms.maps.model.BitmapDescriptorFactory;
import com.google.android.gms.maps.model.CameraPosition;
import com.google.android.gms.maps.model.LatLng;
import com.google.android.gms.maps.model.Marker;
import com.google.android.gms.maps.model.MarkerOptions;

public class MapOverlay
{
	
	// Our own map types, used for a common interface between Google and Apple maps:
	final static public int MAP_TYPE_NORMAL = 0;
	final static public int MAP_TYPE_SATELLITE = 1;
	final static public int MAP_TYPE_HYBRID = 2;
	final static public int MAP_TYPE_TERRAIN = 3;
	final static public int MAP_TYPE_MAX = 4;
	
	final static public int MAP_NO_MARKER_CLICKED = -1;
	final static public double MAP_INVALID_COORDINATE = 1000.0f;
	
	final static public float MAP_MARKER_RED = 0.0f;
	final static public float MAP_MARKER_ORANGE = 22.5f;
	final static public float MAP_MARKER_YELLOW = 45.0f;
	final static public float MAP_MARKER_GREEN = 90.0f;
	final static public float MAP_MARKER_BLUE = 180.0f;
	final static public float MAP_MARKER_PURPLE = 270.0f;
	
	// Table for mapping our own map types to equivalent GoogleMap types:
	final static private int mMapTypeToGoogleMapType[] =
	{
		GoogleMap.MAP_TYPE_NORMAL,
		GoogleMap.MAP_TYPE_SATELLITE,
		GoogleMap.MAP_TYPE_HYBRID,
		GoogleMap.MAP_TYPE_TERRAIN
	};
		
	
	
	private int mMapX = 0;
	private int mMapY = 0;
	private int mMapWidth = 800;
	private int mMapHeight = 400;
	private RelativeLayout mFullScreenLayout = null;
	private GoogleMap mGoogleMap = null;
	private RelativeLayout mMapLayout = null;
	private FragmentActivity mParent = null;
	private final int mMapLayoutID = 99001;
	private ArrayList<Marker> mMarkerArray = new ArrayList<Marker>();
	private ArrayList<Double> mMarkerLatitudeArray = new ArrayList<Double>();
	private ArrayList<Double> mMarkerLongitudeArray = new ArrayList<Double>();
	private ArrayList<String> mMarkerTitleArray = new ArrayList<String>();
	private int mNumMarkers = 0;
	private int mNumMarkerAdditionsPending = 0;
	private boolean mMapNeedsConnection = false;
	private FragmentTransaction mFragmentTransaction = null;
	private FragmentManager mFragmentManager = null;
	private SupportMapFragment mMapFragment = null;
	private double mLatitude = 39.5f;
	private double mLongitude = -98.0f;
	private int mClickedMarkerIndex = MAP_NO_MARKER_CLICKED;
	private boolean mMapHasMoved = false;
	private boolean mMapClicked = false;
	private boolean mClickListenerSet = false;
	private double mMapClickLatitude = MAP_INVALID_COORDINATE;
	private double mMapClickLongitude = MAP_INVALID_COORDINATE;
	private double mMapCenterLatitude = MAP_INVALID_COORDINATE;
	private double mMapCenterLongitude = MAP_INVALID_COORDINATE;


	public MapOverlay(FragmentActivity parent)
	{
		mParent = parent;
		
		mFullScreenLayout = new RelativeLayout(mParent);
		
		mMapLayout = new RelativeLayout(mParent);
		mMapLayout.setId(mMapLayoutID);

		RelativeLayout.LayoutParams layout_params = new RelativeLayout.LayoutParams(mMapWidth,mMapHeight);
		layout_params.setMargins(mMapX, mMapY, 0, 0);
		mMapLayout.setLayoutParams(layout_params);
		
		ViewGroup view = (ViewGroup) parent.findViewById(android.R.id.content);
		mFullScreenLayout.addView(mMapLayout);
		view.addView(mFullScreenLayout);

		mFragmentManager = mParent.getSupportFragmentManager();

		GoogleMapOptions gmo = new GoogleMapOptions();
		gmo.useViewLifecycleInFragment(false);

		mMapFragment = SupportMapFragment.newInstance(gmo);
		
		mMapNeedsConnection = true;

		// Get the map from the SupportMapFragment - This may not work the first time, so it will
		// use recursion with a delay until it succeeds:
		getMapFromFragment();
		
		// Attach the map to the map layout:
		mFragmentTransaction = mFragmentManager.beginTransaction();
		mFragmentTransaction.add(mMapLayout.getId(), mMapFragment);				
		mFragmentTransaction.commitAllowingStateLoss();
				
		return;
		
	}
	
	public void hide()
	{
		mMapLayout.setVisibility(View.GONE);
	}
	
	public void show()
	{
		mMapLayout.setVisibility(View.VISIBLE);
	}

	public void setDimensions(int width, int height)
	{
		mMapWidth = width;
		mMapHeight = height;
		RelativeLayout.LayoutParams layout_params = new RelativeLayout.LayoutParams(mMapWidth,mMapHeight);
		layout_params.setMargins(mMapX, mMapY, 0, 0);
		mMapLayout.setLayoutParams(layout_params);
	}

	public void setPosition(int x, int y)
	{
		mMapX = x;
		mMapY = y;
		RelativeLayout.LayoutParams layout_params = (RelativeLayout.LayoutParams) mMapLayout.getLayoutParams();
		layout_params.setMargins(mMapX, mMapY, 0, 0);
		mMapLayout.setLayoutParams(layout_params);
	}
	
	public void setCenterCoordinates(double latitude, double longitude)
	{
		if (mGoogleMap != null && latitude >= -180.0f && latitude <= 180 && longitude >= -180.0f && longitude <= 180f)
		{
			mLatitude = latitude;
			mLongitude = longitude;
			
			LatLng ll = new LatLng(mLatitude, mLongitude);
			mGoogleMap.moveCamera(CameraUpdateFactory.newLatLng(ll));
		}

	}
	
	public void setZoom(float zoom)
	{
		if (mGoogleMap != null)
		{
			// Note: not checking or enforcing limits of zoom value
			mGoogleMap.moveCamera(CameraUpdateFactory.zoomTo(zoom));
		}
	}

	public void setMyLocationEnabled(boolean enabled)
	{
		if (mGoogleMap != null)
		{
			// Note: not checking or enforcing limits of zoom value
			mGoogleMap.setMyLocationEnabled(enabled);
		}
	}

	public void setClickListener()
	{
		if (mGoogleMap != null && !mClickListenerSet)
		{
			mClickListenerSet = true;
			mGoogleMap.setOnMapClickListener(new OnMapClickListener()
			{
	
				@Override
				public void onMapClick(LatLng point)
				{
					mMapClicked = true;
					mMapClickLatitude = point.latitude;
					mMapClickLongitude = point.longitude;
				}
	
			});
		}
	}

	public void update()
	{
		// Do things on the UI thread so values are ready to return for non-UI thread methods
		if (mGoogleMap != null)
		{
			mMapCenterLatitude = mGoogleMap.getCameraPosition().target.latitude;
			mMapCenterLongitude = mGoogleMap.getCameraPosition().target.longitude;
		}
	}
	
	public boolean mapClicked()
	{
		// Might not have set the click listener yet - Can't set it until the map has initialized itself. Make sure it's set.
		
		boolean return_value = mMapClicked;
		mMapClicked = false;
		return(return_value);
	}
	
	public double getMapClickLatitude()
	{
		return(mMapClickLatitude);
	}
	
	public double getMapClickLongitude()
	{
		return(mMapClickLongitude);
	}
	
	private void setMoveListener()
	{
		if (mGoogleMap != null)
		{
			mGoogleMap.setOnCameraChangeListener(new OnCameraChangeListener()
			{
				@Override
				public void onCameraChange(CameraPosition position)
				{ // TODO Auto-generated method stub

					mMapHasMoved = true;

				}
			}); // end of setOnCameraChangeListener
		}
	}
	
	public boolean hasMoved()
	{
		boolean return_value = mMapHasMoved;
		mMapHasMoved = false; // consume the move
		return(return_value);
	}
	
	public void setType(int map_type)
	{
		if (map_type >= 0 && map_type <= MAP_TYPE_MAX)
		{
			// Map our own map type to the equivalent GoogleMaps type:
			int google_map_type = mMapTypeToGoogleMapType[map_type];
			mGoogleMap.setMapType(google_map_type);
		}
	}

	public double getCenterLatitude() //returns latitude and longitude of center of map
	{
		return (mMapCenterLatitude); 
	}

	public double getCenterLongitude() //returns latitude and longitude of center of map
	{
		return (mMapCenterLongitude); 
	}
	
	/* Methods for handling markers: */
	
	public void clear()
	{
		mMarkerArray.clear();
		mMarkerLatitudeArray.clear();
		mMarkerLongitudeArray.clear();
		mMarkerTitleArray.clear();
		if (mGoogleMap != null)
		{
			mGoogleMap.clear();
		}
		mNumMarkers = 0;
	}
	
	public int addMarker(double latitude, double longitude, String title)
	{
		mNumMarkers++;
		mNumMarkerAdditionsPending = mNumMarkerAdditionsPending - 1; 
		int new_marker_idx = mNumMarkers - 1;
		Marker new_marker = mGoogleMap.addMarker(new MarkerOptions()
			.position(new LatLng(latitude, longitude))
			.title(title)
			.icon(BitmapDescriptorFactory.defaultMarker(0.0f))); // 0.0f - defaults hue to red
		mMarkerArray.add(new_marker_idx, new_marker);
		mMarkerLatitudeArray.add(new_marker_idx, latitude);
		mMarkerLongitudeArray.add(new_marker_idx, longitude);
		mMarkerTitleArray.add(new_marker_idx, title);
		
		return (new_marker_idx);
	}
	
	public void setMarkerTitle(int marker_idx, String title)
	{
		if (marker_idx >= 0 && marker_idx < mNumMarkers)
		{
			mMarkerArray.get(marker_idx).setTitle(title);
			mMarkerTitleArray.set(marker_idx, title);
		}
	}
	
	public void setMarkerHue(int marker_idx, float hue)
	{
		if (marker_idx >= 0 && marker_idx < mNumMarkers)
		{
			mMarkerArray.get(marker_idx).setIcon(BitmapDescriptorFactory.defaultMarker(hue));
		}
	}
	
	public void setMarkerAlpha(int marker_idx, float alpha)
	{
		if (marker_idx >= 0 && marker_idx < mNumMarkers)
		{
			mMarkerArray.get(marker_idx).setAlpha(alpha / 100.0f);
		}
	}

	public void setMarkerCoordinates(int marker_idx, double latitude, double longitude)
	{
		if (marker_idx >= 0 && marker_idx < mNumMarkers)
		{
			LatLng lat_lon = new LatLng(latitude, longitude);			
			mMarkerArray.get(marker_idx).setPosition(lat_lon);
			mMarkerLatitudeArray.set(marker_idx, latitude);
			mMarkerLongitudeArray.set(marker_idx, longitude);
		}
	}
	
	public String getMarkerTitle(int marker_idx)
	{
		if (marker_idx >= 0 && marker_idx < mNumMarkers)
		{
			return(mMarkerTitleArray.get(marker_idx));
		}
		return null;
	}
	
	public double getMarkerLatitude(int marker_idx)
	{
		if (marker_idx >= 0 && marker_idx < mNumMarkers)
		{
			return(mMarkerLatitudeArray.get(marker_idx));
		}
		return MAP_INVALID_COORDINATE;
	}

	public double getMarkerLongitude(int marker_idx)
	{
		if (marker_idx >= 0 && marker_idx < mNumMarkers)
		{
			return(mMarkerLongitudeArray.get(marker_idx));
		}
		return MAP_INVALID_COORDINATE;
	}
	
	public int getClickedMarkerIndex()
	{
		// Can call this function periodically (even every frame) to see if a marker has been clicked since the last call.
		// If so, it will return its index.  
		
		int return_value = mClickedMarkerIndex;
		
		// Consume the click, if any:
		mClickedMarkerIndex = MAP_NO_MARKER_CLICKED;
		return(return_value);
	}

	private void getMapFromFragment()
	{
		if (mMapNeedsConnection)
		{
			if (mMapFragment != null)
			{
				mGoogleMap = mMapFragment.getMap();
				// The map fragment may not be ready to return its map yet. See if we got a map: 
				if (mGoogleMap == null)
				{
					// Check back after a slight delay:
					Handler h = new Handler();
					h.postDelayed(new Runnable()
						{ public void run() { getMapFromFragment(); } }, 250); // 250 ms delay
					return;
				}

				// We have a map - set it up:
				mGoogleMap.setMyLocationEnabled(true);
				
				// mMap.setOnMyLocationButtonClickListener(this);

				if (mLatitude < -360.0f || mLatitude > 360)
				{
					mLatitude = 39.5f;
					mLongitude = -98.0f;
				}

				CameraPosition cp = mGoogleMap.getCameraPosition();
				double latitude = cp.target.latitude;
				double longitude = cp.target.longitude;
				LatLng ll = new LatLng(mLatitude, mLongitude);

				// For some strange reason, setting the map position doesn't
				// work for latitude on the first try. Maybe the map isn't
				// entirely ready?
				int tries = 0;
				while (latitude != mLatitude && tries++ < 100)
				{
					//mAutoMapCameraMovePending = true;
					//mNumCameraMovesPending++;
					mGoogleMap.moveCamera(CameraUpdateFactory.newLatLng(ll));
					cp = mGoogleMap.getCameraPosition();
					latitude = cp.target.latitude;
				}

				//add_map_content();
				setMoveListener();
				mGoogleMap.setOnMarkerClickListener
				(
					new OnMarkerClickListener()
					{
			
						@Override
						public boolean onMarkerClick(Marker marker)
						{
							String marker_id = marker.getId();
	
							// Find the marker that was clicked in our array, and store the index					
							int i;
							mClickedMarkerIndex = MAP_NO_MARKER_CLICKED;
	
							for (i = 0; i < mNumMarkers; i++)
							{
								if (mMarkerArray.get(i).getId().contentEquals(marker_id))
								{
									mClickedMarkerIndex = i;
									i = mNumMarkers;
								}
							}
							marker.showInfoWindow();
							return true;
						} // end of on marker click
					} // end of setOnMarkerClickListener 
				);


				mGoogleMap.setOnCameraChangeListener(new OnCameraChangeListener()
				{

					@Override
					public void onCameraChange(CameraPosition position)
					{ // TODO Auto-generated method stub
					}
				} // end of setOnCameraChangeListener
				);

				mMapNeedsConnection = false;
				return;
			}
		}
		else // map doesn't need connection
		{
			CameraPosition cp = mGoogleMap.getCameraPosition();
			double latitude = cp.target.latitude;
			double longitude = cp.target.longitude;
		}
	}
	
	
	public int getNextAddedMarkerIndex()
	{
		int result = mNumMarkers + mNumMarkerAdditionsPending;
		
		mNumMarkerAdditionsPending = mNumMarkerAdditionsPending + 1;
		
		return result;
		
	}
	
} // end of Map class

