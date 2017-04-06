<?php

function islandora_datastreams_io_export_batch_form(array $form, array &$form_state, $batch = NULL) {
  dpm('solr = [' . $solr . ']');
  dpm('upitt_workflow_batch = [' . $upitt_workflow_batch . ']');
  module_load_include('inc', 'upitt_workflow', 'includes/utilities');

  $fetch_methods = array(0 => 'Solr query', 1 => 'Batch CSV', 2 => 'Collection', 3 => 'Model');
  if (!module_exists('islandora_solr')) {
    unset($fetch_methods[0]);
  }

  $batch_id = ((is_numeric($batch)) ? $batch : upitt_workflow_get_batch_id($batch, FALSE));
  if ($batch_id) {
    $batch = upitt_workflow_batch_load($batch_id);
    $csv_arr = upitt_workflow_csv_file_as_array($batch['file']);
    $pid_values = upitt_workflow_csv_extract_identifiers($csv_arr);
    $pids = array();
    foreach ($pid_values as $p) {
      $pids[]['PID'] = $p;
    }
  }

  $batch_path = ($batch_id ? upitt_workflow_batch_export_path($batch['batch_external_id']) : 'batch_id_NOT_SET');

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

  $markup_export = '<p>' . t('This will allow a specific datastream from a given set of objects to be exported to a folder under ' .
          '<code>' . $batch_path . '</code>.  A set of objects can be defined by a Solr query, by collection membership, by content ' .
          'model, or by the identifiers of the CSV associated with a batch.') . '</p>';

  $form['exportform'] = array(
    'form_description' => array('#markup' => $markup_export),
    'breadcrumb' => array('#markup' => upitt_workflow_get_breadcrumb_path()),

    'pid_tmp_file' => array(
      '#type' => 'hidden',
      '#default_value' => '/tmp/batch_export_' . date('YmdHis') . '_' . $batch_id,
    ),
    'batch_id' => array(
      '#type' => 'hidden',
      '#default_value' => $batch_id,
    ),
    'pids' => array(
      '#type' => 'hidden',
      '#default_value' => implode('
', $pid_values),
    ),

    'datastreamformset' => array(
      '#type' => 'fieldset',
      '#title' => t('Datastream'),
      '#collapsible' => TRUE,
      '#description' => t('Select which datastream to export.'),
      '#group' => 'exportform',

      'datastream' => array(
        '#type' => 'select',
        '#description' => 'Select datastream',
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
      '#description' => t('For the export, select only one method to select the objects.'),

      'pids_fetch_method' => array(
        '#type' => 'radios',
        '#options' => $fetch_methods,
        '#default_value' => (isset($form_state['input']['pids_fetch_method']) ? $form_state['input']['pids_fetch_method'] : ''),
      ),

      'solr_query' => array(
        '#type' => 'textarea',
        '#title' => t('Solr query'),
        '#default_value' => (isset($form_state['input']['solr_query']) ? $form_state['input']['solr_query'] : ''),
        '#group' => 'exportform',
      ),

      'batch_csv' => array(
        '#type' => 'textfield',
        '#title' => t('Batch'),
        '#attributes' => array('readonly' => 'readonly', 'disabled' => 'disabled'),
        '#description' => t('Batch CSV file') . ((count($pids) > 0) ? theme('table', array('rows'=>$pids,'header'=>array(0=>'PID'),'attributes' =>array('class'=>array('report_table')))) : ''),
        '#default_value' => ($batch && isset($batch['file']))? $batch['file'] : '',
        '#group' => 'exportform',
      ),

      'collection' => array(
        '#type' => 'select',
        '#description' => 'Select collection',
        '#title' => t('Collection'),
        '#options' => $collections,
        '#default_value' => (isset($form_state['input']['collection']) ? $form_state['input']['collection'] : ''),
        '#size' => 10,
        '#group' => 'exportform',
      ),

      'model' => array(
        '#type' => 'select',
        '#description' => 'Select model',
        '#title' => t('Model'),
        '#options' => $model_options,
        '#default_value' => (isset($form_state['input']['model']) ? $form_state['input']['model'] : ''),
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

function islandora_datastreams_io_export_batch_form_submit(array $form, array &$form_state) {
  global $user;

  $pids_fetch_method = $form_state['values']['pids_fetch_method'];
  $batch_id = $form_state['values']['batch_id'];
  $pids_file = $form_state['values']['pid_tmp_file'];
  $pids = NULL;

  $batch_path = ($batch_id ? upitt_workflow_batch_export_path($batch['batch_external_id']) : 'batch_id_NOT_SET');
  $export_csv_file = $batch_path . '/csv_export_file.csv';
  
  // Depending on the pids_fetch_method value, the PID values are derived one of 4 different ways:
  switch ($pids_fetch_method) {
    // 0 - Solr query
    case 0:
      $solr_query = $form_state['values']['solr_query'];
      drupal_set_message('Lookup PIDS for Solr query: "' . $solr_query . '"');
      $pids = upitt_workflow_get_pids_solr_query($solr_query);
      break;

    // 1 - Batch CSV
    case 1:
      $pids = $form_state['values']['pids'];
      drupal_set_message('PIDS from batch spreadsheet: "' . $pids . '"');
      break;

    // 2 - Collection
    case 2:
      $collection = $form_state['values']['collection'];
      drupal_set_message('Lookup PIDS for Collection: "' . $collection . '"');
      $pids = upitt_workflow_get_pids_collection($collection);
      break;

    // 3 - Model
    case 3:
      $model = $form_state['values']['model'];
      drupal_set_message('Lookup PIDS for fedora Model: "' . $model . '"');
      $pids = upitt_workflow_get_pids_model($model);
      break;
  }
  if ($pids) {
    $rm_command = 'rm /tmp/batch_export_*_' . $batch_id;
    exec($rm_command);
    $this_domain = 'http://' . $_SERVER['HTTP_HOST'];

    file_put_contents($pids_file, $pids);
    $datastream = $form_state['values']['datastream'];
    $drush_command = 'drush islandora_datastream_crud_fetch_datastreams -u ' . $user->uid .
                     ' --pid_file=' . $pids_file . ' --dsid=' . $datastream .
                     ' --datastreams_directory=' . $batch_path .
                     ' --silent=1 ' .
                     ' --uri=' . $this_domain;
    $output = $return_var = array();
    exec($drush_command, $output, $return_var);
    if ($return_var == -1 || $return_var == 255) {
      drupal_set_message('There was an error calling the drush command : ' . $drush_command);
    }
  }
  else {
    drupal_set_message(t('There were no PID values for this export.  ' . $pids_file . ' Please try again.'));
  }

  //  $form_state['redirect'] = '/workflow/batch/' . $batch_id;
}

