<?php

function islandora_datastreams_io_relationships_form(array $form, array &$form_state, $batch = NULL) {
  module_load_include('inc', 'islandora_datastreams_io', 'includes/utilities');

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

  $relationship_options = array('add' => 'Add relationship', 'remove' => 'Remove relationship');
  $default_pids_value = implode('
', $pid_values);
  $fetch_methods = islandora_datastreams_io_get_fetch_methods($pid_values);
  $tmp_model_options = islandora_datastreams_io_get_models();

  $tmp_collections = islandora_datastreams_io_get_solr_options('RELS_EXT_hasModel_uri_ms:info\:fedora\/islandora\:collectionCModel', 'PID', 'fgs_label_s');
  asort($tmp_collections);
  $collections = array();
  foreach ($tmp_collections as $k => $collection) {
    $collections['info:fedora/' . $k] = $collection;
  }

  foreach ($tmp_model_options as $k => $model) {
    $model_options['info:fedora/' . $k] = $model;
  }

  $markup_export = '<p>' . t('This will allow a specific datastream from a given set of objects to be downloaded. ' .
          'A set of objects can be defined by a Solr query, by collection membership, by content ' .
          'model, or by a list of PID values.') . '</p>';

  if (array_key_exists('values', $form_state)) {
    $markup_export .= islandora_datastreams_io_pids_report_markup($form_state['values']);
  }

  $form['relationshipsform'] = array(
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
        '#group' => 'relationshipsform',
      ),
      'solr_query' => array(
        '#type' => 'textarea',
        '#title' => t('Solr query'),
        '#default_value' => (isset($form_state['input']['solr_query']) ? $form_state['input']['solr_query'] : $default_pids_value),
        '#description' => 'Use the query "q" section of any Solr query - ' .
          l(variable_get('islandora_solr_url', 'localhost:8080/solr'), variable_get('islandora_solr_url', 'localhost:8080/solr')) .
          ' do not try to use filter queries "fq" or field lists "fl" or any special Solr options for this query.',
        '#group' => 'relationshipsform',
      ),
      'collection' => array(
        '#type' => 'select',
        '#description' => 'Select collection',
        '#title' => t('Collection'),
        '#options' => $collections,
        '#default_value' => (isset($form_state['input']['collection']) ? $form_state['input']['collection'] : $default_pids_value),
        '#size' => 10,
        '#group' => 'relationshipsform',
      ),
      'model' => array(
        '#type' => 'select',
        '#description' => 'Select model',
        '#title' => t('Model'),
        '#options' => $model_options,
        '#default_value' => (isset($form_state['input']['model']) ? $form_state['input']['model'] : $default_pids_value),
        '#size' => 7,
        '#group' => 'relationshipsform',
      ),
    ),
    'operationformset' => array(
      '#type' => 'fieldset',
      '#title' => t('Operation'),
      '#collapsible' => TRUE,
      '#description' => t('Select the operation to perform on all matched objects.'),
      '#group' => 'relationshipsform',
      'operation' => array(
        '#type' => 'select',
        '#description' => 'Select operation to perform on the matched objects.',
        '#title' => t('Operation'),
        '#options' => $relationship_options,
        '#default_value' => (isset($form_state['input']['operation']) ? $form_state['input']['operation'] : ''),
        '#size' => 7,
        '#group' => 'relationshipsform',
        '#required' => TRUE,
      ),
      'namespace' => array(
        '#type' => 'textfield',
        '#title' => t('Namespace of relationship ontology'),
        '#description' => t('The namespace of the relationship.  For example, most relationships use the ' .
                'namespace value of "' . FEDORA_RELS_EXT_URI . '", but for the isMemberOfSite relationship ' .
                'the namespace value must be "http://digital.library.pitt.edu/ontology/relations#".'),
        '#default_value' => FEDORA_RELS_EXT_URI,
      ),
      'predicate' => array(
        '#type' => 'textfield',
        '#title' => t('Predicate value for the relationship'),
        '#description' => t('The predicate is the name of the relationship.  For example, "isMemberOf" or "isMemberOfCollection".'),
        '#default_value' => '',
        '#required' => TRUE,
      ),
      'value' => array(
        '#type' => 'textfield',
        '#title' => t('Value'),
        '#description' => t('When the `value` is a fedora object, the prefix "info:fedora/" should not be provided.  This value ' . 
                'can be empty for the purpose of setting an empty value - which is <b>not the same as removing the relationship.</b>.'),
        '#default_value' => '',
      ),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Update Relationships'),
    ),
    'see_selected' => array(
      '#type' => 'submit',
      '#value' => t('See Selected Objects'),
    ),
  );

  return $form;
}

function islandora_datastreams_io_relationships_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_datastreams_io', 'includes/utilities');

  $pids_fetch_method = $form_state['values']['pids_fetch_method'];
  $pids = NULL;

  // Unset the _SESSION value here.
  if (isset($_SESSION['pids_value']) && isset($_SESSION['pids_fetch_method'])) {
    unset($_SESSION['pids_value']);
    unset($_SESSION['pids_fetch_method']);
  }

  $fetch_message = '';
  // Depending on the pids_fetch_method value, the PID values are derived one of 4 different ways:
  switch ($pids_fetch_method) {
    // 0 - List of PIDS
    case ISLANDORA_DATASTREAMS_IO_FETCH_LISTPIDS:
      $pids = $form_state['values']['list_of_pids'];
      $fetch_message = 'PIDS provided by user';
      break;

    // 1 - Solr query
    case ISLANDORA_DATASTREAMS_IO_FETCH_SOLR:
      $solr_query = $form_state['values']['solr_query'];
      $fetch_message = 'Lookup PIDS for Solr query: "' . $solr_query . '"';
      $pids = islandora_datastreams_io_get_pids_solr_query($solr_query);
      break;

    // 2 - Collection
    case ISLANDORA_DATASTREAMS_IO_FETCH_COLLECTION:
      $collection = $form_state['values']['collection'];
      $fetch_message = 'Lookup PIDS for Collection: "' . $collection . '"';
      $pids = islandora_datastreams_io_get_pids_collection($collection);
      break;

    // 3 - Model
    case ISLANDORA_DATASTREAMS_IO_FETCH_MODEL:
      $model = $form_state['values']['model'];
      $fetch_message = 'Lookup PIDS for fedora Model: "' . $model . '"';
      $pids = islandora_datastreams_io_get_pids_model($model);
      break;
  }

  if ($fetch_message) {
    drupal_set_message($fetch_message);
  }
  if ($pids) {
    $bad_pids = $good_pids = $relationship_skipped = array();
    $updated_count = 0;
    $pids_arr = explode("\n", $pids);
    $operation = $form_state['values']['operation'];
    $namespace = $form_state['values']['namespace'];
    $predicate = $form_state['values']['predicate'];
    $value = $form_state['values']['value'];
    foreach ($pids_arr as $pid) {
      $pid = str_replace("\\r", "", trim($pid));
      $islandora_object = islandora_object_load($pid);
      if (is_object($islandora_object)) {
        $good_pids[] = $pid;
        // ONLY do this if the triggering element was not the button to
        if ($form_state['triggering_element']['#value'] <> 'See Selected Objects') {
          $relationships = $islandora_object->relationships->get($namespace, $predicate);
          $found = false;
          foreach ($relationships as $relationship) {
            $found |= ($relationship['object']['value'] == $value);
          }
          if ($operation == 'add') {
            if (!$found) {
              $islandora_object->relationships->add($namespace, $predicate, $value);
              $updated_count++;
            }
            else {
              $relationship_skipped[] = $pid;
            }
          }
          elseif ($operation == 'remove') {
            if ($found) {
              $islandora_object->relationships->remove($namespace, $predicate, $value);
              $updated_count++;
            }
            else {
              $relationship_skipped[] = $pid;
            }
          }
        }
      }
      else {
        $bad_pids[] = $pid;
      }
    }
    if ($form_state['triggering_element']['#value'] <> 'See Selected Objects') {
      if (count($relationship_skipped) > 0) {
        drupal_set_message('At least one of relationships for the objects did not need to be changed.  These are: \'' . implode("', '", $relationship_skipped) . '\'', 'info');
      }
      drupal_set_message('The relationship of "' . $predicate . '" = "' . $value . '" has been ' . (($operation == 'remove') ? 'removed' : 'added') . ' for the objects.');
    }
    else {
      file_put_contents("temporary://selected_pids", implode("\n", $good_pids));
      $form_state['rebuild'] = TRUE;
    }
    if (count($bad_pids) > 0) {
      drupal_set_message('At least one of the PID values were not able to be loaded as an object.  These are: <ul>' . implode("</li></li>", $bad_pids) . '</ul>', 'warning');
    }
  }
  else {
    drupal_set_message(t('There were no PID values for this export.  Please try again.'));
  }
}

