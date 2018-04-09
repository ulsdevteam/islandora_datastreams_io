<?php

function islandora_datastreams_io_get_exported_files($dir) {
  $files = array();
  $cdir = scandir($dir);
  foreach ($cdir as $key => $value) {
    if (!in_array($value,array(".",".."))) {
      if (($value[0] <> '.') && ($value <> '__MACOSX')) {
        $files[] = $value;
      }
    }
  }
  return $files;
}

function islandora_datastreams_io_ds_file_parts($filename) {
  $last_ = strrchr($filename, "_");
  if (strlen($last_)) {
    $dsid_ext = substr($filename, (-strlen($last_) + 1));
    $ns_pid = substr($filename, 0, (strlen($filename) - strlen($last_)));

    @list($namespace, $pid) = explode("_", $filename, 2);
    @list($dsid, $ext) = explode(".", $dsid_ext);

    $pid = str_replace("_" . $dsid . "." . $ext, "", $pid);
    return array('namespace' => $namespace, 'pid' => $namespace . ':' . $pid, 'dsid' => $dsid, 'extension' => $ext);
  }
  else {
    return array();
  }
}

/**
 * Delete the temporary uncompressed folder from zip file upload.
 */
function islandora_datastreams_io_dump_temp_dir($full_extract_path) {
  if ($full_extract_path <> '.') {
    $files = glob(rtrim($full_extract_path, '/') . '/*');
    foreach($files as $file){
      if(is_file($file))
        unlink($file);
    }
    // since Apple OS adds extra files to zip files, a manually created ZIP from
    // them would not be empty and rmdir() would fail... this is BRUTE force.
    islandora_datastreams_io_deleteDirectory($full_extract_path);
  }
}

function islandora_datastreams_io_deleteDirectory($dir) {
  if ($dir && $dir <> '.') {
    system('rm -rf ' . escapeshellarg($dir), $retval);
    return $retval == 0; // UNIX commands return zero on success
  }
}

function islandora_datastreams_io_get_pids_solr_query($solr_query) {
  return implode('
', islandora_datastreams_io_get_pids($solr_query));
}

function islandora_datastreams_io_get_pids_collection($collection) {
  $query = format_string('!field:!value', array(
    '!field' => 'RELS_EXT_isMemberOfCollection_uri_ms',
    '!value' => str_replace(array("/", ":", "-", "+"), array("\/", "\:", "\-", "\+"), $collection),
   ));
  return implode('
', islandora_datastreams_io_get_pids($query));
}

function islandora_datastreams_io_get_pids_model($model) {
  $query = format_string('!field:!value', array(
    '!field' => 'RELS_EXT_hasModel_uri_ms',
    '!value' => str_replace(array("/", ":", "-", "+"), array("\/", "\:", "\-", "\+"), $model),
   ));
  return implode('
', islandora_datastreams_io_get_pids($query));
}

function islandora_datastreams_io_get_pids($query) {
  module_load_include('inc', 'islandora_solr', 'includes/utilities');
  $query_processor = new IslandoraSolrQueryProcessor();

  $ret_arr = array();
  // run two queries --
  // 1) to get the datastreams available on this object
  $query_processor->solrQuery = $query;
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = variable_get('islandora_datastreams_io_solr_limit', 10000);
  $query_processor->solrParams = array('fl' => 'PID');

  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);
  try {
    $search_results = $solr->search($query_processor->solrQuery, $query_processor->solrStart, $query_processor->solrLimit, $query_processor->solrParams, 'GET');
    $tmp = json_decode($search_results->getRawResponse(), TRUE);

    $results = array();
    $numFound = $tmp['response']['numFound'];
    if ($numFound == variable_get('islandora_datastreams_io_solr_limit', 10000)) {
      drupal_set_message(t('The number of objects returned is the same as the maximum limit.  It is likely that there are more objects than ' . variable_get('islandora_datastreams_io_solr_limit', 10000)));
    }

    if ($tmp['response']['numFound'] > 0) {
      foreach ($tmp['response']['docs'] as $k=>$rec) {
        $ret_arr[] = $rec['PID'];
      }
    }
  }
  catch (Exception $e) {
    error_log('EXCEPTION in islandora_datastreams_io_get_pids : called from ' . $_SERVER['SERVER_NAME'] .
' - ' . $_SERVER['REQUEST_URI'] . '
' . print_r($e, true));
  }
  return $ret_arr;
}

function islandora_datastreams_io_get_models() {
  form_load_include($form_state, 'inc', 'islandora', 'includes/content_model.autocomplete');

  // Retrieve all content models and unset those currently in use in this
  // configuration and any others from other configurations.
  $add_options = islandora_get_content_model_names();
  return $add_options;
}

/**
 * Helper function that will return the list of datastreams that are currently in
 * use from running a Solr query.
 *
 * @return string
 */
function islandora_datastreams_io_get_datastreams($pid = '') {
  $query_processor = new IslandoraSolrQueryProcessor();
  if ($pid) {
    $query_processor->solrQuery = 'PID:' . str_replace(array("/", ":", "-", "+"), array("\/", "\:", "\-", "\+"), $pid);
  } else {
    $query_processor->solrQuery = '*:*';
  }
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 0;
  $query_processor->solrParams = array('facet' => 'true',
                                       'facet.query' => 'fedora_datastreams_ms:["" TO *]',
                                       'facet.field' => 'fedora_datastreams_ms');

  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);
  $results = array();
  try {
    $search_results = $solr->search($query_processor->solrQuery, $query_processor->solrStart, $query_processor->solrLimit, $query_processor->solrParams, 'GET');
    $tmp = json_decode($search_results->getRawResponse(), TRUE);
    if ($tmp['response']['numFound'] > 0) {
      foreach ($tmp['facet_counts']['facet_fields']['fedora_datastreams_ms'] as $key => $num) {
        $results[$key] = $key . ' (' . $num . ')';
      }
    }
    ksort($results);
  }
  catch (Exception $e) {
  }
  return $results;
}

/**
 * Helper function that will take the datastreams that are returned by the function
 * islandora_datastreams_io_get_datastreams -- and inspect them to see whether or not
 * the datastreams are a "text/xml" mimetype.  This is needed to present the user with
 * a set of datastreams that can have a transform run against them.
 *
 * @param array $datastreams
 * @param type $pid
 * @return array
 */
function islandora_datastreams_io_textxml_datastreams($datastreams, $pid = '') {
  $query_processor = new IslandoraSolrQueryProcessor();
  $keep_dsids = array();
  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);

  // check the datastream fedora_datastream_latest_OBJ_MIMETYPE_ms: has values that are "text/xml"
  foreach ($datastreams as $dsid => $dsid_desc) {
    $mimetype_field = 'fedora_datastream_latest_' .$dsid . '_MIMETYPE_ms';
    if ($pid) {
      $query_processor->solrQuery = 'PID:' . str_replace(array("/", ":", "-", "+"), array("\/", "\:", "\-", "\+"), $pid) . ' AND ' . $mimetype_field . ':text\/xml';
    } else {
      $query_processor->solrQuery = $mimetype_field . ':text\/xml' ;
    }
    $query_processor->solrStart = 0;
    $query_processor->solrLimit = 1;
    $query_processor->solrParams = array('');

    $results = array();
    try {
      $search_results = $solr->search($query_processor->solrQuery, $query_processor->solrStart, $query_processor->solrLimit, $query_processor->solrParams, 'GET');
      $tmp = json_decode($search_results->getRawResponse(), TRUE);
      if ($tmp['response']['numFound'] > 0) {
        $keep_dsids[$dsid] = $dsid . ' (' . $tmp['response']['numFound'] . ')';
      }
    }
    catch (Exception $e) {
    }
  }
  return $keep_dsids;
}

function islandora_datastreams_io_get_solr_options($solr_query, $key_field, $value_field, $sort_field = '') {
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = $solr_query;
  if ($sort_field) {
    $query_processor->sort = $sort_field;
  }
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 40000;
  $query_processor->solrParams = array('fl' => $key_field . ',' . $value_field);

  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);
  $results = array();
  try {
    $search_results = $solr->search($query_processor->solrQuery, $query_processor->solrStart, $query_processor->solrLimit, $query_processor->solrParams, 'GET');
    $tmp = json_decode($search_results->getRawResponse(), TRUE);
    if ($tmp['response']['numFound'] > 0) {
      foreach ($tmp['response']['docs'] as $rec) {
        $results[$rec[$key_field]] = $rec[$value_field];
      }
    }
  }
  catch (Exception $e) {
  }
  return $results;
}

/**
 * This will return the mimetype for a given filename.
 *
 * @param string $file
 *   A filename.
 * @return string
 *   The mimetype that corresponds to the $file provided.
 */
function islandora_datastreams_io_mimetype_of_file($file) {
  require_once DRUPAL_ROOT . "/includes/file.mimetypes.inc";
  $mimetype_mapping = file_mimetype_mapping();
  $file_pathinfo = pathinfo($file);
  $fallback_mimetype = 'text/xml';
  if (isset($file_pathinfo['extension'])) {
    $lowercase_extension = strtolower($file_pathinfo['extension']);
    $mimetype_index = isset($mimetype_mapping['extensions'][$lowercase_extension]) ? $mimetype_mapping['extensions'][$lowercase_extension] : FALSE;
    if (!($mimetype_index === FALSE)) {
      return $mimetype_mapping['mimetypes'][$mimetype_index];
    }
    else {
      return $fallback_mimetype;
    }
  }
  else {
    return $fallback_mimetype;
  }
}

/**
 * Will return the PID values that are namespace accessible.
 *
 * @param type $pids
 * @return string
 *   PID values that are namespace accessible for the islandora site.
 */
function islandora_datastreams_io_pids_namespace_accessible($pids) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $inspect_pids = explode("\n", $pids);
  $return_pids = array();
  foreach ($inspect_pids as $pid) {
    $pid = trim($pid);
    if (islandora_namespace_accessible($pid)) {
      $return_pids[] = $pid;
    }
    else {
      drupal_set_message('PID ' . $pid . ' is restricted and will be skipped.');
    }
  }
  return implode("\n", $return_pids);
}

function islandora_datastreams_io_file_put_contents($filename, $data) {
  $f = @fopen($filename, 'wb');
  if ($f === FALSE) {
    return false;
  } else {
    $bytes = fwrite($f, $data);
    fclose($f);
    return $bytes;
  }
}

function islandora_datastreams_io_file_get_contents($filename) {
  $rh = fopen($filename, 'rb');
  if ($rh === FALSE) {
    return false;
  } else {
    $out = '';
    while (!feof($rh)) {
      $out .= fread($rh, 1024);
    }
    fclose($rh);
    return $out;
  }
}

function islandora_datastreams_io_get_mods_schema_versions() {
  return array(
    '' => 'hook_mods_schema_check ONLY',
    'https://www.loc.gov/standards/mods/v3/mods-3-7.xsd' => 'MODS 3.7 Schema',
    'https://www.loc.gov/standards/mods/v3/mods-3-6.xsd' => 'MODS 3.6',
    'https://www.loc.gov/standards/mods/v3/mods-3-5.xsd' => 'MODS 3.5',
    'https://www.loc.gov/standards/mods/v3/mods-3-4.xsd' => 'MODS 3.4',
    'https://www.loc.gov/standards/mods/v3/mods-3-3.xsd' => 'MODS 3.3',
    'https://www.loc.gov/standards/mods/v3/mods-3-2.xsd' => 'MODS 3.2',
    'https://www.loc.gov/standards/mods/v3/mods-3-1.xsd' => 'MODS 3.1',
    'https://www.loc.gov/standards/mods/v3/mods-3-0.xsd' => 'MODS 3.0'
  );
}

function islandora_datastreams_io_mods_schema_success($import_mods_filename, $schema_uri) {
  // Potentially handle the schema check with any module/s that have coded for 
  // a hook_mods_schema_check method.
  $retval = TRUE;
  foreach (module_implements('mods_schema_check') as $module) {
    $function = $module . '_mods_schema_check';
    // will call all modules implementing hook_mods_schema_check
    // and can pass each argument as reference determined
    // by the function declaration
    $retval &= $function($import_mods_filename);
  }
  // If there was a hook_mods_schema_check in any module, $retval holds the
  // Success / Failure of that function/s.  If the $schema_uri is not set,
  // ONLY the value of $retval at this point would be returned (meaning that
  // a configuration that has "NONE" selected for the "MODS Schema" and no
  // hook_mods_schema_check coded in any installed modules, the return code
  // would be TRUE).
  if (!$schema_uri) {
    return $retval;
  }

  // First, check the local temp directory to see if the MODS schema has been
  // stored locally.  If not, get it and store it there for quicker access
  // for subsequent schema tests.
  $path_info = pathinfo($schema_uri);
  $local_schema_filename = "temporary://" . $path_info['basename'];
  if (!file_exists($local_schema_filename)) {
    $schema_contents = file_get_contents($schema_uri);
    file_put_contents($local_schema_filename, $schema_contents);
  }
  // Get the datatream from the islandora_object -- and perform the schema check.
  try {
    $mods_content = file_get_contents($import_mods_filename);
    $doc0 = new DOMDocument();
    if (!(@$doc0->loadXML($mods_content) === false)) {
      $retval = ($doc0->schemaValidate($local_schema_filename));
    } else {
      $retval = FALSE;
    }
  }
  catch (Exception $e) {
     die('EXCEPTION in islandora_datastreams_io_schema_success : called from ' . $_SERVER['SERVER_NAME'] .
  ' - ' . $_SERVER['REQUEST_URI'] . '<pre>
  ' . print_r($e, true));
  }
  return $retval;
}

function islandora_datastreams_io_get_fetch_methods($pid_values) {
  $fetch_methods = array(
      ISLANDORA_DATASTREAMS_IO_FETCH_LISTPIDS => 'List of PID values' .
        ((count($pid_values) > 0) ? ' (' . count($pid_values) . ' PID value' .
        ((count($pid_values) == 1) ? '' : 's') . ' supplied)' : ''));
  if (module_exists('islandora_solr')) {
    $fetch_methods[ISLANDORA_DATASTREAMS_IO_FETCH_SOLR] = 'Solr query';
    $fetch_methods[ISLANDORA_DATASTREAMS_IO_FETCH_COLLECTION] = 'Collection';
    $fetch_methods[ISLANDORA_DATASTREAMS_IO_FETCH_MODEL] = 'Model';
  }
  return $fetch_methods;
}

function islandora_datastreams_io_pids_report_markup($form_values) {
  $temp_pids_filename = "temporary://selected_pids";
  if (file_exists($temp_pids_filename)) {
    switch ($form_values['pids_fetch_method']) {
      case ISLANDORA_DATASTREAMS_IO_FETCH_LISTPIDS:
        $fetch_message = 'PIDS provided by user';
        break;
      case ISLANDORA_DATASTREAMS_IO_FETCH_SOLR:
        $solr_query = $form_values['solr_query'];
        $fetch_message = 'Lookup PIDS for Solr query: "' . $solr_query . '"';
        break;
      case ISLANDORA_DATASTREAMS_IO_FETCH_COLLECTION:
        $collection = $form_values['collection'];
        $fetch_message = 'Lookup PIDS for Collection: "' . $collection . '"';
        break;
      case ISLANDORA_DATASTREAMS_IO_FETCH_MODEL:
        $model = $form_values['model'];
        $fetch_message = 'Lookup PIDS for fedora Model: "' . $model . '"';
        break;
    }
    $temp_pids_file = file_get_contents($temp_pids_filename);
    $temp_pids_file_as_arr = explode("\n", $temp_pids_file);

    foreach ($temp_pids_file_as_arr as $key => $pid) {
      $temp_pids_file_as_arr[$key] = l($pid, 'islandora/object/' . $pid,
          array('attributes'=>array('target' => '_blank')));
    }
    unlink($temp_pids_filename);
    return '<div class="datastreams_io_report"><h3>Selected PID values (' .
      $fetch_message . ')</h3>Represents ' . number_format(count($temp_pids_file_as_arr)).
      ' Islandora objects<pre>' . implode("<br>", $temp_pids_file_as_arr) . '</pre></div>';
  }
  else {
    return '<div class="datastreams_io_report"><h3>Selected PID values (' .
      $fetch_message . ')</h3>It appears that the form was not submitted or that no PID values ' .
        'resulted.  "' . $temp_pids_filename . '" did not exist.</div>';
  }
}
