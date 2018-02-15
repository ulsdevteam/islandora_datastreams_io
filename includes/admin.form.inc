<?php

/**
 * @file
 * Administration forms and submit handling for Islandora Datastreams Input/Output & Object Relationships module.
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
  global $user;

  $has_import_access = user_access(ISLANDORA_DATASTREAMS_IO_IMPORT, $user);

  module_load_include('inc', 'islandora_datastream_crud', 'islandora_datastream_crud.drush');
  $islandora_datastream_crud_drush_commands = @islandora_datastream_crud_drush_command();
  $bold_commands = array('islandora_datastream_crud_fetch_datastreams', 'islandora_datastream_crud_push_datastreams');
  foreach ($islandora_datastream_crud_drush_commands as $command => $command_params) {
    $bold_wrap_pre = ((array_search($command, $bold_commands) === FALSE) ? '' : '<strong>');
    $bold_wrap_post = ((array_search($command, $bold_commands) === FALSE) ? '' : '</strong>');
    $commands[$command] = $bold_wrap_pre . $command . $bold_wrap_post . (isset($command_params['description']) ? '<br>' . $command_params['description'] : '');
  }

  $about_islandora_datastreams_io_markup = '<p>' . l('Export','admin/islandora/datastreams_io/export'). ' - export a ZIP ' .
          'file of object datastreams.</p>' .
          ($has_import_access ? '<p>' . l('Import','admin/islandora/datastreams_io/import'). ' - upload ZIP of object datastreams.</p>' : '') .
          ($has_import_access ? '<p>' . l('Relationships','admin/islandora/datastreams_io/relationships'). ' - add / remove a relationship to a set of objects.</p>' : '') .
          ($has_import_access ? '<p>' . l('XSLT Transform','admin/islandora/datastreams_io/transform'). ' - run an xslt transform on datastreams for a set of objects.  You can self transform, create a copy of a datastream, or create derived datastreams this way.</p>' : '') .
          '<p>' . l('Update Object label from MODS titleInfo/title','admin/islandora/datastreams_io/update_label'). ' - update object label value from MODS for specific objects.</p>' .
          '<p>' . l('Purge Datastreams','admin/islandora/datastreams_io/delete_datastreams'). ' - delete specific datastreams from objects.</p>' .
          '<p>' . l('Generate Datastream Derivatives','admin/islandora/datastreams_io/generate_derivatives'). ' - generate derivatives for sepecific datastreams on specific objects.</p>' .
          '<blockquote><p>This module provides a convenient web ' .
          'wrapper for several of the drush functions provided by islandora_datastream_crud.</p>' .
          '<ul><li>' . implode('</li><li>', $commands) . '</li></ul></blockquote>';

  if (module_exists('xml_form_builder')) {
    module_load_include('inc', 'xml_form_builder', 'includes/associations');
    $transforms = xml_form_builder_get_transforms($associations);
    $transforms_k_v = array();
    // Why does this seem to be using the value for the keys?
    foreach ($transforms as $k => $v) {
      $transforms_k_v[$v] = $k;
    }
    $mods_to_dc_desc = 'The xslt transform to convert MODS to DC. Leave blank if any resultant MODS should not generate DC.';
  }
  else {
    $transforms['No Transform'] = 'No Transform';
    $mods_to_dc_desc = 'The islandora_xml_forms module is not installed.  Install it to be able to use the "mods_to_dc.xsl" transforms that are configured there.';
  }

  $form = array(
    'mods_to_dc_transform' => array(
      '#type' => 'select',
      '#options' => $transforms_k_v,
      '#default_value' => variable_get('islandora_datastreams_io_mods_to_dc_transform', 'No Transform'),
      '#title' => 'MODS to DC XSL Transform',
      '#description' => $mods_to_dc_desc,
    ),
    'frame' => array(
      '#type' => 'fieldset',
      '#title' => 'About Islandora Datastreams Input/Output & Object Relationships',
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
      variable_set('islandora_datastreams_io_mods_to_dc_transform', $form_state['values']['mods_to_dc_transform']);
      drupal_goto('/admin/islandora');
      break;
    case 'edit-reset':
      variable_del('islandora_datastreams_io_mods_to_dc_transform');
      break;
  }
}
