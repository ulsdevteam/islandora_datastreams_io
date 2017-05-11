<?php

function islandora_datastreams_io_export_form(array $form, array &$form_state, $batch = NULL) {
  module_load_include('inc', 'upitt_workflow', 'includes/utilities');

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

  $tmp_model_options = upitt_workflow_get_models();
  $datastreams = upitt_workflow_get_datastreams();

  $tmp_collections = upitt_workflow_get_solr_options('RELS_EXT_hasModel_uri_ms:info\:fedora\/islandora\:collectionCModel', 'PID', 'fgs_label_s');
  asort($tmp_collections);

  foreach ($tmp_collections as $k => $collection) {
    $collections['info:fedora/' . $k] = $collection;
  }

  foreach ($tmp_model_options as $k => $model) {
    $model_options['info:fedora/' . $k] = $model;
  }

  $markup_export = '<p>' . t('This will allow a specific datastream from a given set of objects to be downloaded. ' .
          'A set of objects can be defined by a Solr query, by collection membership, by content ' .
          'model, or by a list of PID values.') . '</p>';

  $form['exportform'] = array(
    'form_description' => array('#markup' => $markup_export),
    'breadcrumb' => array('#markup' => upitt_workflow_get_breadcrumb_path()),

    'pids' => array(
      '#type' => 'hidden',
      '#default_value' => $default_pids_value,
    ),

    'datastreamformset' => array(
      '#type' => 'fieldset',
      '#title' => t('Datastream'),
      '#collapsible' => TRUE,
      '#description' => t('Select which datastream to export.  '),
      '#group' => 'exportform',

      'datastream' => array(
        '#type' => 'select',
        '#description' => 'Select datastream to export.  It is possible to export ' .
          'fewer datastreams than objects because not all objects have all datastreams.<ul>' .
          '<li>It may take a very long time to export a large number of objects or ' .
          'to export larger datastreams such as OBJ, JPG, AVI, MP4, etc.</li></ul>',
        '#title' => t('Datastream'),
        '#options' => $datastreams,
        '#default_value' => (isset($form_state['input']['datastream']) ? $form_state['input']['datastream'] : ''),
        '#size' => 7,
        '#group' => 'exportform',
      ),
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
        '#group' => 'exportform',
      ),

      'list_of_pids' => array(
        '#type' => 'textarea',
        '#title' => t('PID values'),
        '#attributes' => $pids_attributes,
        '#description' => t('PID values, one on each line.'),
        '#default_value' => $default_pids_value,
        '#group' => 'exportform',
      ),

      'collection' => array(
        '#type' => 'select',
        '#description' => 'Select collection',
        '#title' => t('Collection'),
        '#options' => $collections,
        '#default_value' => (isset($form_state['input']['collection']) ? $form_state['input']['collection'] : $default_pids_value),
        '#size' => 10,
        '#group' => 'exportform',
      ),

      'model' => array(
        '#type' => 'select',
        '#description' => 'Select model',
        '#title' => t('Model'),
        '#options' => $model_options,
        '#default_value' => (isset($form_state['input']['model']) ? $form_state['input']['model'] : $default_pids_value),
        '#size' => 7,
        '#group' => 'exportform',
      ),
    ),

    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Export batch'),
    ),
  );

  return $form;
}

function islandora_datastreams_io_export_form_submit(array $form, array &$form_state) {
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
      $pids = upitt_workflow_get_pids_solr_query($solr_query);
      break;

    // 1 - List of PIDS
    case ISLANDORA_DATASTREAMS_IO_FETCH_LISTPIDS:
      $pids = $form_state['values']['list_of_pids'];
      break;

    // 2 - Collection
    case ISLANDORA_DATASTREAMS_IO_FETCH_COLLECTION:
      $collection = $form_state['values']['collection'];
      drupal_set_message('Lookup PIDS for Collection: "' . $collection . '"');
      $pids = upitt_workflow_get_pids_collection($collection);
      break;

    // 3 - Model
    case ISLANDORA_DATASTREAMS_IO_FETCH_MODEL:
      $model = $form_state['values']['model'];
      drupal_set_message('Lookup PIDS for fedora Model: "' . $model . '"');
      $pids = upitt_workflow_get_pids_model($model);
      break;
  }
  if ($pids) {
    // This process may take more time to complete.
    set_time_limit(0);
    $export_path = 'datastream_io_datastreams_' . date('His_hms');
    chdir(file_directory_temp());
    mkdir($export_path);
    $full_export_path = file_directory_temp() . '/' . $export_path;
    $zip_name = $full_export_path . '/file.zip';

    $this_domain = 'http://' . $_SERVER['HTTP_HOST'];

    $script_base = str_replace('/index.php', '', $_SERVER['SCRIPT_FILENAME']);
    $SETTINGS_PATH = $script_base . '/' . conf_path(TRUE);

    chdir($SETTINGS_PATH);

    $pids_file = tempnam(file_directory_temp(), 'pids_file.txt');
    file_put_contents($pids_file, $pids);
    $datastream = $form_state['values']['datastream'];
    $drush_command = 'drush islandora_datastream_crud_fetch_datastreams -u ' . $user->uid .
                     ' --pid_file=' . $pids_file . ' --dsid=' . $datastream .
                     ' --datastreams_directory=' . $full_export_path .
                     ' --silent=1 ' .
                     ' --uri=' . $this_domain;
    $output = $return_var = array();
    exec($drush_command, $output, $return_var);
    if ($return_var == -1 || $return_var == 255) {
      drupal_set_message('There was an error calling the drush command : ' . $drush_command);
    }

    $files = islandora_datastreams_io_get_exported_files($full_export_path);
    chdir($full_export_path);
    $zip = new ZipArchive;
    $zip->open($zip_name, ZipArchive::CREATE);
    foreach ($files as $file) {
      $zip->addFile($file);
    }
    $zip->close();

    if (file_exists($zip_name)) {
      $base64_zip_name = base64_encode($zip_name);
      $zip_size = file_exists($zip_name) ? filesize($zip_name) : 0;
      $download_link = l('Download ZIP export', 'islandora/datastreams_io/download_export/' . $base64_zip_name);

      drupal_set_message('The export of datastreams is complete.  ' . $download_link . ' ' . islandora_datastreams_io_human_filesize($zip_size));
    }
    else {
      drupal_set_message('The zip file could not be created.  It is possible that there were no matching datastreams for the selected objects', 'warning');
    }

    // Remove the temporary files
    @unlink($pids_file);
  }
  else {
    drupal_set_message(t('There were no PID values for this export.  Please try again.'));
  }
}

function islandora_datastreams_io_human_filesize($bytes, $decimals = 2) {
  $sz = 'BKMGTP';
  $factor = floor((strlen($bytes) - 1) / 3);
  return sprintf("%.{$decimals}f", $bytes / pow(1024, $factor)) . @$sz[$factor];
}
