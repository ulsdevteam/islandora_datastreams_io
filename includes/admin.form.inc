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
  module_load_include('inc', 'islandora_datastream_crud', 'islandora_datastream_crud.drush');
  $islandora_datastream_crud_drush_commands = @islandora_datastream_crud_drush_command();
  $bold_commands = array('islandora_datastream_crud_fetch_datastreams', 'islandora_datastream_crud_push_datastreams');
  foreach ($islandora_datastream_crud_drush_commands as $command => $command_params) {
    $bold_wrap_pre = ((array_search($command, $bold_commands) === FALSE) ? '' : '<strong>');
    $bold_wrap_post = ((array_search($command, $bold_commands) === FALSE) ? '' : '</strong>');
    $commands[$command] = $bold_wrap_pre . $command . $bold_wrap_post . (isset($command_params['description']) ? '<br>' . $command_params['description'] : '');
  }

  $about_islandora_datastreams_io_markup = '<p>' . l('Export','islandora/datastreams_io/export'). ' - export a ZIP ' .
          'file of object datastreams.</p>' .
          '<p>' . l('Import','islandora/datastreams_io/import'). ' - upload ZIP of object datastreams.</p>' .
          '<p>This module provides a convenient web ' .
          'wrapper for several of the drush functions provided by islandora_datastream_crud.</p>' .
          '<ul><li>' . implode('</li><li>', $commands) . '</li></ul>';

  $form = array(
    'frame' => array(
      '#type' => 'fieldset',
      '#title' => 'About Islandora Datastreams I/O',
      'about' => array(
        '#type' => 'item',
        '#markup' => $about_islandora_datastreams_io_markup,
      )
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Return'),
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
  $id = $form_state['triggering_element']['#id'];
  switch ($id) {
    case 'edit-submit':
      drupal_goto('/admin/islandora');
      break;

    case 'edit-reset':
      break;
  }
}
