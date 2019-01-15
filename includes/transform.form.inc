<?php

function islandora_datastreams_io_transform_form(array $form, array &$form_state, $batch = NULL) {
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
  $fetch_methods = islandora_datastreams_io_get_fetch_methods($pid_values);
  $tmp_model_options = islandora_datastreams_io_get_models();
  $all_datastreams = islandora_datastreams_io_get_datastreams();
  $datastreams = islandora_datastreams_io_textxml_datastreams($all_datastreams);

  // This will loop through all datastreams and add an asterisk to the ones
  // that are not part of the textxml mimetype list of datastreams.  This "*"
  // character is used in the javascript to disable the file upload button
  // for selecting a transform.
  foreach ($all_datastreams as $dsid => $dsid_label_count) {
    if (!(array_key_exists($dsid, $datastreams))) {
      unset($datastreams[$dsid]);
    }
  }

  $extensions = array('xsl','xslt');
  
  $tmp_collections = islandora_datastreams_io_get_solr_options('RELS_EXT_hasModel_uri_ms:info\:fedora\/islandora\:collectionCModel', 'PID', 'fgs_label_s');
  asort($tmp_collections);
  $collections = array();
  foreach ($tmp_collections as $k => $collection) {
    $collections['info:fedora/' . $k] = $collection . ' (' . $k . ')';
  }

  foreach ($tmp_model_options as $k => $model) {
    $model_options['info:fedora/' . $k] = $model;
  }

  $markup_export = '<p>' . t('This will allow a specific datastream from a ' .
    'given set of objects to be downloaded. A set of objects can be defined ' .
    'by a Solr query, by collection membership, by content model, or by a ' .
    'list of PID values.') . '</p>';

  $form['transformsform'] = array(
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
        '#default_value' => ((isset($form_state['input']['pids_fetch_method']) &&
          ($form_state['input']['pids_fetch_method']) <> '') ?
          $form_state['input']['pids_fetch_method'] : ''),
      ),
      'list_of_pids' => array(
        '#type' => 'textarea',
        '#title' => t('PID values'),
        '#attributes' => $pids_attributes,
        '#description' => t('PID values, one on each line.'),
        '#default_value' => $default_pids_value,
        '#group' => 'transformsform',
      ),
      'solr_query' => array(
        '#type' => 'textarea',
        '#title' => t('Solr query'),
        '#default_value' => (isset($form_state['input']['solr_query']) ? $form_state['input']['solr_query'] : $default_pids_value),
        '#description' => 'Use the query "q" section of any Solr query - ' .
          l(variable_get('islandora_solr_url', 'localhost:8080/solr'), variable_get('islandora_solr_url', 'localhost:8080/solr')) .
          ' do not try to use filter queries "fq" or field lists "fl" or any special Solr options for this query.',
        '#group' => 'transformsform',
      ),
      'collection' => array(
        '#type' => 'select',
        '#description' => 'Select collection',
        '#title' => t('Collection'),
        '#options' => $collections,
        '#default_value' => (isset($form_state['input']['collection']) ? $form_state['input']['collection'] : $default_pids_value),
        '#size' => 10,
        '#group' => 'transformsform',
      ),
      'model' => array(
        '#type' => 'select',
        '#description' => 'Select model',
        '#title' => t('Model'),
        '#options' => $model_options,
        '#default_value' => (isset($form_state['input']['model']) ? $form_state['input']['model'] : $default_pids_value),
        '#size' => 7,
        '#group' => 'transformsform',
      ),
    ),
    'datastreamformset' => array(
      '#type' => 'fieldset',
      '#title' => t('Datastream'),
      '#collapsible' => TRUE,
      '#description' => t('Select which datastream to transform.  Since some ' .
          'objects may not have the selected datastream, it is possible to ' .
          'transform fewer datastreams than the number of selected objects. ' .
          'Transforms are only available on datastreams that have a mimetype ' .
          'of "text/xml".'),
      '#group' => 'transformsform',
      'datastream' => array(
        '#type' => 'select',
        '#title' => t('Datastream'),
        '#options' => $datastreams,
        '#default_value' => (isset($form_state['input']['datastream']) ? $form_state['input']['datastream'] : ''),
        '#size' => 7,
        '#required' => TRUE,
        '#group' => 'exportform',
      ),
      'selected_dsid' => array(
        '#type' => 'hidden',
      ),
    ),
    'transformformset' => array(
      '#type' => 'fieldset',
      '#title' => t('Transform'),
      '#collapsible' => TRUE,
      '#description' => t('Select the transform to run on all matched ' . 
          'objects.  If you want to copy the source datastream as a new ' .
          'datastream (without any change), do not upload a transform.  ' .
          'Transforms can only run on datastreams with a mimetype of ' .
          '"text/xml".'),
      '#group' => 'transformsform',
      'transform' => array(
        '#type' => 'file',
        '#title' => t('XSLT Transform'),
        '#description' => t('Select the XSLT Transform to run against the ' .
          'selected datastreams.<br/>Files must be less than <strong>@size ' .
          'MB.</strong><br/>Allowed file types: <strong>@ext.</strong>',
                array('@size' => $upload_size, '@ext' => $extensions[0])),
        '#upload_location' => 'temporary://',
        '#upload_validators' => array(
          'file_validate_extensions' => $extensions,
           // Assume its specified in MB.
          'file_validate_size' => array($upload_size * 1024 * 1024),
        ),
      ),
    ),
    'selftransformformset' => array(
      '#type' => 'fieldset',
      '#title' => t('Destination Datastream'),
      '#collapsible' => TRUE,
      '#description' => t('If this is a self transform, the resultant ' .
          'transformed file will be saved over the original source.  ' .
          'Non-existant datastreams can be created by entering a new ' .
          'datastream identifier.'),
      '#group' => 'transformsform',
      'destination_dsid' => array(
        '#type' => 'textfield',
      ),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Update'),
    ),
  );

  return $form;
}

function islandora_datastreams_io_transform_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_datastreams_io', 'includes/utilities');
  global $user;

  $pids_fetch_method = $form_state['values']['pids_fetch_method'];
  $pids = NULL;
  $selected_dsid = $form_state['values']['selected_dsid'];

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

    $transform = '';
    // ONLY use the $_FILE if the $selected_dsid does not contain "*"
    if (strstr($selected_dsid, "*") == '') {
      if (isset($_FILES['files']['name']['transform']) && isset($_FILES['files']['size']['transform'])) {
        if ($_FILES['files']['type']['transform'] == 'text/xml') {
          $transform = $_FILES['files']['tmp_name']['transform'];
        }
        elseif (isset($_FILES['files']['type']['transform']) && $_FILES['files']['type']['transform']) {
          drupal_set_message('The transform file was not a "text/xml" type of file and can not be used.' . $_FILES['files']['type']['transform'], 'warning');
        }
      }
    }
    $dsid = $form_state['values']['datastream'];
    $destination_dsid = $form_state['values']['destination_dsid'];
    foreach ($pids_arr as $pid) {
      $pid = trim($pid);
      $islandora_object = islandora_object_load($pid);
      if (is_object($islandora_object)) {
        // run the $transform on the datastream

        if (islandora_datastreams_io_transform_datastream($islandora_object, $dsid, $destination_dsid, $transform)) {
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

function islandora_datastreams_io_transform_datastream($islandora_object, $dsid, $destination_dsid, $transform = '') {
  $transform_xsl = $transform;
  // If there is a value set for $destination_dsid, use it -- else this is
  // a self-transform.
  $destination_dsid = ($destination_dsid) ? $destination_dsid : $dsid;

  if (isset($islandora_object[$dsid])) {
    // Get the object's datastream saved to a temp file
    $pid_filename = str_replace(":", "_", $islandora_object->id) . '_' . $dsid;
    $xml_file = tempnam(file_directory_temp(), $pid_filename);
    $got_datastream = $islandora_object[$dsid]->getContent($xml_file);
    if ($got_datastream && $transform) {
      $new_ds_content = islandora_datastreams_io_runXslTransform(
          array(
            'xsl' => $transform_xsl,
            'input' => islandora_datastreams_io_file_get_contents($xml_file),
          )
        );
      // This file must be deleted in the process function that called this.
      $bytes_written = islandora_datastreams_io_file_put_contents($xml_file, $new_ds_content);
      drupal_set_message('bytes = ' . $bytes_written);
    } elseif ($got_datastream) {
      $new_ds_content = islandora_datastreams_io_file_get_contents($xml_file);
    }
    // create the new datastream if needed
    if (!isset($islandora_object[$destination_dsid])) {
      $datastream = $islandora_object->constructDatastream($destination_dsid);
      $datastream->label = $islandora_object[$dsid]->label . ' {copied from ' . $dsid . '}';
      $datastream->mimeType = $islandora_object[$dsid]->mimeType;
      $datastream->setContentFromString($new_ds_content);
      $islandora_object->ingestDatastream($datastream);
    }
    else {
      $islandora_object[$destination_dsid]->setContentFromString($new_ds_content);
    }

    if (!$got_datastream) {
      drupal_set_message('The datastream could not be retrieved for object ' . $islandora_object->id . '.', 'warning');
    }
    return ($got_datastream && (isset($islandora_object[$dsid])));
  }
  else {
    drupal_set_message('There is no "' . $dsid . '" datastream for object ' . $islandora_object->id . '.', 'warning');
  }
}

function islandora_datastreams_io_runXslTransform($info) {
  $xsl = new DOMDocument();
  $xsl->load($info['xsl']);
  $input = new DOMDocument();
  $input->loadXML($info['input']);

  $processor = new XSLTProcessor();
  $processor->importStylesheet($xsl);

/*
  if (isset($info['php_functions'])) {
    $processor->registerPHPFunctions($info['php_functions']);
  }
*/

  // XXX: Suppressing warnings regarding unregistered prefixes.
  return $processor->transformToXML($input);
}
