/* the old hook menu and settings pages:

$items[] = array(
  'path' => 'admin/settings/geo',
  'title' => t('Geo'),
  'callback' => 'drupal_get_form',
  'callback arguments' => array('geo_admin_settings'),
  'access' => user_access('administer site configuration'),
  'type' => MENU_NORMAL_ITEM
  'discription' => t('Configure settings for the geo module'),
);

function geo_admin_settings() {
  $location_fields = _geo_location_fields();
  
  $form['geo_geocode_fields'] = array (
    '#type' => 'checkboxes',
    '#title' => t('Geocoding Fields'),
    '#description' => t('Select which fields to pass to Google for geocoding. More fields may provide more accuracy in some geographic loactions.'),
    '#options' => $location_fields,
    '#default_value' => variable_get('geo_geocode_fields', array_keys($location_fields)),
  );
  return system_settings_form($form);
}

*/

function geo_nodeapi(&$node, $op, $a3 = NULL, $a4 = NULL) {

  if ($op == 'submit') {

    if(is_array($node->locations)) {

      foreach($node->locations as $key => $location) {

        // create a crippled location using only the fields whitelisted on the settings page
        $crippled_location = array();
        $fields_to_use = variable_get('geo_geocode_fields', array_keys(_geo_location_fields()));
        foreach($fields_to_use as $subfield) {
          
          // provinces come with the country code prepended, strip 'em
          if($subfield == 'province') {
            $crippled_location[$subfield] = substr($location[$subfield], 3);
          } else {
            $crippled_location[$subfield] = $location[$subfield];
          }
        }

        // we need to geocode via google
        $point = geo_geocode($crippled_location);
        
        // update the node
        $node->locations[$key]['latitude'] = $point['lat'];
        $node->locations[$key]['longitude'] = $point['lon'];
      }
    }
  }
}

// stolen from location.us.inc because the old version insisted on too much accuracy
function geo_geocode($location = array()) {
  $service_url = 'http://maps.google.com/maps/geo?output=xml&key='. variable_get('googlemap_api_key', '') .'&q=';

  $address = location_address2singleline($location);
  
  $http_reply = drupal_http_request($service_url . urlencode($address));
  
  $status_code_match = array();
  preg_match('/<code>(.*)<\/code>/', $http_reply->data, $status_code_match);
  $status_code = $status_code_match[1];
  if ($status_code != 200) {
    
    return NULL;
  }
  
  $accuracy_code_match = array();
  preg_match('/Accuracy="([0-9])"/', $http_reply->data, $accuracy_code_match);
  $accuracy_code = $accuracy_code_match[1];
  // someday may implement methods of discarding inaccurate results. for now, we keep
  // if ($accuracy_code != 8 && $accuracy_code != 7) {
  //   dprint_r("Acurracy code was: $accuracy_code");
  //   return NULL;
  // }
  
  $latlon_match = array();
  preg_match('/<coordinates>(.*)<\/coordinates>/', $http_reply->data, $latlon_match);
  
  $latlon_exploded = explode(',', $latlon_match[1]);
  
  return array('lat' => $latlon_exploded[1], 'lon' => $latlon_exploded[0]);
}

// little helper function to hold the keys and names of the location fields
function _geo_location_fields() {
  return array (
    'street' => t('Street'),
    'city' => t('City'),
    'province' => t('State/Province'),
    'postal_code' => t('Postal Code'),
    'country' => t('Country'),
  );
}
