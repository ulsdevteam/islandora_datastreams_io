<?php

function islandora_datastreams_io_purge_datastreams_form(array $form, array &$form_state, $batch = NULL) {
  module_load_include('inc', 'islandora_datastreams_io', 'includes/utilities');

  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $pids_attributes = array();
  $pids_fetch_method = isset($_SESSION['pids_fetch_method']) ? $_SESSION['pids_fetch_method'] : ISLANDORA_DATASTREAMS_IO_FETCH_LISTPIDS;

  $pid_values = (isset($_SESSION['pids_value']) && $pids_fetch_method == ISLANDORA_DATASTREAMS_IO_FETCH_LISTPIDS) ?
    json_decode($_SESSION['pids_value'], true) :
    array();
  // This will set the radio button to the appropriate choice for fetch method, and
  // the javascript will hide / show the appropriate form elements.
  if (isset($_SESSION['pids_value']) && $pids_fetch_method == ISLANDORA_DATASTREAMS_IO_FETCH_LISTPIDS) {
    $form_state['input']['pids_fetch_method'] = $pids_fetch_method;
    $pids_attributes['readonly'] = 'readonly';
    $pids_attributes['disabled'] = 'disabled';
  }

  $default_pids_value = implode('
', $pid_values);
  $fetch_methods = array(ISLANDORA_DATASTREAMS_IO_FETCH_SOLR => 'Solr query',
      ISLANDORA_DATASTREAMS_IO_FETCH_LISTPIDS => 'List of PID values' .
        ((count($pid_values) > 0) ? ' (' . count($pid_values) . ' PID value' . ((count($pid_values) == 1) ? '' : 's') . ' supplied)' : ''),
      ISLANDORA_DATASTREAMS_IO_FETCH_COLLECTION => 'Collection',
      ISLANDORA_DATASTREAMS_IO_FETCH_MODEL => 'Model');
  if (!module_exists('islandora_solr')) {
    unset($fetch_methods[0]);
  }

  $tmp_model_options = islandora_datastreams_io_get_models();
  $datastreams = islandora_datastreams_io_get_datastreams();
  $datastreams = islandora_datastreams_io_textxml_datastreams($datastreams);

  $extensions = array('xsl','xslt');
  
  $tmp_collections = islandora_datastreams_io_get_solr_options('RELS_EXT_hasModel_uri_ms:info\:fedora\/islandora\:collectionCModel', 'PID', 'fgs_label_s');
  asort($tmp_collections);
  $collections = array();
  foreach ($tmp_collections as $k => $collection) {
    $collections['info:fedora/' . $k] = $collection;
  }

  foreach ($tmp_model_options as $k => $model) {
    $model_options['info:fedora/' . $k] = $model;
  }

  $markup_export = '<p>' . t('This will allow a specific datastream from a given set ' .
          'of objects to be purged. A set of objects can be defined by a Solr query, ' .
          'by collection membership, by content model, or by a list of PID values.') . '</p>';

  $form['purgedsform'] = array(
    'form_description' => array('#markup' => $markup_export),

    'pids' => array(
      '#type' => 'hidden',
      '#default_value' => $default_pids_value,
    ),

    'objectsformset' => array(
      '#type' => 'fieldset',
      '#title' => t('Select objects'),
      '#collapsible' => TRUE,
      '#description' => t('For the export, select the method to select the objects.'),

      'pids_fetch_method' => array(
        '#type' => 'radios',
        '#options' => $fetch_methods,
        '#default_value' => (isset($form_state['input']['pids_fetch_method']) ? $form_state['input']['pids_fetch_method'] : ''),
      ),

      'solr_query' => array(
        '#type' => 'textarea',
        '#title' => t('Solr query'),
        '#default_value' => (isset($form_state['input']['solr_query']) ? $form_state['input']['solr_query'] : $default_pids_value),
        '#description' => 'Use the query "q" section of any Solr query - ' .
          l(variable_get('islandora_solr_url', 'localhost:8080/solr'), variable_get('islandora_solr_url', 'localhost:8080/solr')) .
          ' do not try to use filter queries "fq" or field lists "fl" or any special Solr options for this query.',
        '#group' => 'purgedsform',
      ),

      'list_of_pids' => array(
        '#type' => 'textarea',
        '#title' => t('PID values'),
        '#attributes' => $pids_attributes,
        '#description' => t('PID values, one on each line.'),
        '#default_value' => $default_pids_value,
        '#group' => 'purgedsform',
      ),

      'collection' => array(
        '#type' => 'select',
        '#description' => 'Select collection',
        '#title' => t('Collection'),
        '#options' => $collections,
        '#default_value' => (isset($form_state['input']['collection']) ? $form_state['input']['collection'] : $default_pids_value),
        '#size' => 10,
        '#group' => 'purgedsform',
      ),

      'model' => array(
        '#type' => 'select',
        '#description' => 'Select model',
        '#title' => t('Model'),
        '#options' => $model_options,
        '#default_value' => (isset($form_state['input']['model']) ? $form_state['input']['model'] : $default_pids_value),
        '#size' => 7,
        '#group' => 'purgedsform',
      ),
    ),
      
    'datastreamformset' => array(
      '#type' => 'fieldset',
      '#title' => t('Datastream'),
      '#collapsible' => TRUE,
      '#description' => t('Select which datastream to purge.'),
      '#group' => 'purgedsform',

      'datastream' => array(
        '#type' => 'select',
        '#description' => 'Select datastream to purge. It is possible to purge fewer ' .
          'datastreams than objects because not all objects have all datastreams.',
        '#title' => t('Datastream'),
        '#options' => $datastreams,
        '#default_value' => (isset($form_state['input']['datastream']) ? $form_state['input']['datastream'] : ''),
        '#size' => 7,
        '#group' => 'exportform',
      ),
    ),

    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Update'),
    ),
  );

  return $form;
}

function islandora_datastreams_io_purge_datastreams_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_datastreams_io', 'includes/utilities');
  global $user;

  $pids_fetch_method = $form_state['values']['pids_fetch_method'];
  $pids = NULL;

  // Unset the _SESSION value here.
  if (isset($_SESSION['pids_value']) && isset($_SESSION['pids_fetch_method'])) {
    unset($_SESSION['pids_value']);
    unset($_SESSION['pids_fetch_method']);
  }

  // Depending on the pids_fetch_method value, the PID values are derived one of 4 different ways:
  switch ($pids_fetch_method) {
    // 0 - Solr query
    case ISLANDORA_DATASTREAMS_IO_FETCH_SOLR:
      $solr_query = $form_state['values']['solr_query'];
      drupal_set_message('Lookup PIDS for Solr query: "' . $solr_query . '"');
      $pids = islandora_datastreams_io_get_pids_solr_query($solr_query);
      break;

    // 1 - List of PIDS
    case ISLANDORA_DATASTREAMS_IO_FETCH_LISTPIDS:
      $pids = $form_state['values']['list_of_pids'];
      break;

    // 2 - Collection
    case ISLANDORA_DATASTREAMS_IO_FETCH_COLLECTION:
      $collection = $form_state['values']['collection'];
      drupal_set_message('Lookup PIDS for Collection: "' . $collection . '"');
      $pids = islandora_datastreams_io_get_pids_collection($collection);
      break;

    // 3 - Model
    case ISLANDORA_DATASTREAMS_IO_FETCH_MODEL:
      $model = $form_state['values']['model'];
      drupal_set_message('Lookup PIDS for fedora Model: "' . $model . '"');
      $pids = islandora_datastreams_io_get_pids_model($model);
      break;
  }
  if ($pids) {
    $bad_pids = $skipped = array();
    $updated_count = 0;
    $pids_arr = explode("\r\n", $pids);

    $dsid = $form_state['values']['datastream'];
    $destination_dsid = $form_state['values']['destination_dsid'];
    foreach ($pids_arr as $pid) {
      $pid = trim($pid);
      $islandora_object = islandora_object_load($pid);
      if (is_object($islandora_object)) {

        if (islandora_datastreams_io_purge_datastream($islandora_object, $dsid)) {
          $updated_count++;
        }
        else {
          $skipped[] = $pid;
        }
      }
      else {
        $bad_pids[] = $pid;
      }
    }
    if (count($skipped) > 0) {
      drupal_set_message('At least one of datastream for the objects could not be transformed.  These are: \'' . implode("', '", $relationship_skipped) . '\'', 'info');
    }
    if (count($bad_pids) > 0) {
      drupal_set_message('At least one of the PID values were not able to be loaded as an object.  These are: \'' . implode("', '", $bad_pids) . '\'', 'warning');
    }
    drupal_set_message('The transform to the ' . $dsid . ' datastream of the objects has been completed.');
  }
  else {
    drupal_set_message(t('There were no PID values for this export.  Please try again.'));
  }
}

function islandora_datastreams_io_purge_datastream($islandora_object, $dsid) {
  if (isset($islandora_object[$dsid])) {
    // Get the object's datastream saved to a temp file
  }
}
