<?php

/**
 * @file
 * Administration forms and submit handling for Islandora Datastreams I/O module.
 */

/**
 * Administration and configuration form for islandora_datastreams_io.
 *
 * @ingroup forms
 *
 * @todo break out non-settings elements into their own form.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_datastreams_io_admin_form() {
  $form = array(
    'incoming_ingest_path' => array(
      '#type' => 'textfield',
      '#title' => t('Input Path (imports)'),
      '#description' => t('The incoming directory.'),
      '#default_value' => variable_get('islandora_datastreams_io_ingest_path', '/ingest/incoming'),
      '#size' => 100,
    ),
    'exports_ingest_path' => array(
      '#type' => 'textfield',
      '#title' => t('Output Path (exports)'),
      '#description' => t('This output directory is where exported datastreams would be saved in a folder corresponding to the date, and the datastream identifier.'),
      '#default_value' => variable_get('islandora_datastreams_io_export_path', '/ingest/tmp'),
      '#size' => 100,
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save configuration'),
    ),
    'reset' => array(
      '#type' => 'submit',
      '#value' => t('Reset to defaults'),
    ),
  );
  return $form;
}

/**
 * Function that sets the Drupal variables with user's input.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_datastreams_io_admin_form_submit(array $form, array &$form_state) {
  drupal_set_message(t('The settings have been updated!'));
  $id = $form_state['triggering_element']['#id'];
  switch ($id) {
    case 'edit-submit':
      variable_set('islandora_datastreams_io_ingest_path', $form_state['values']['incoming_ingest_path']);
      variable_set('islandora_datastreams_io_export_path', $form_state['values']['exports_ingest_path']);
      break;

    case 'edit-reset':
      variable_del('islandora_datastreams_io_ingest_path');
      variable_del('islandora_datastreams_io_export_path');
      break;
  }
}
