<?php

// TODO: add an option to create the datastream if it does not exist for the
// referenced objects.  This would be helpful for adding new datastreams to
// existing objects such as TRANSCRIPT for manuscript pages.

function islandora_datastreams_io_import_form(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_datastreams_io', 'includes/utilities');

  if (!isset($form_state['stage'])) {
    $form_state['stage'] = 'import_pickfile';
  }

  switch ($form_state['stage']) {
    case 'import_pickfile':
      return islandora_datastreams_io_import_pickfile($form, $form_state);
     break;

    case 'review_import_file':
      return islandora_datastreams_io_review_import_file($form, $form_state, FALSE);
     break;

    case 'ok_import_file':
      return islandora_datastreams_io_review_import_file($form, $form_state, TRUE);
     break;
  }
}

function islandora_datastreams_io_import_pickfile($form, &$form_state) {
  $markup_import = '<p>' . t('This will allow a specific datastream from a ' . 
      'given set of objects to be downloaded.  A set of objects can be ' .
      'defined by a Solr query, by collection membership, by content model, ' .
      'or by a list of PID values.') . '</p>';

  // Since this is used to clean up the temp file, and the second and third step 
  // could potentially overwrite this value, make an empty value here - and simply
  // check the value before setting in step 2.
  $_SESSION['archive_file'] = '';
  // Prepare a couple variables for use with file upload options:
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $extensions = array('zip');

  $form['importform'] = array(
    'form_description' => array('#markup' => $markup_import),

    'file' => array(
      '#type' => 'file',
      '#title' => t('File'),
      '#description' => t('Select ZIP file of datastream files.<br/>Files ' . 
          'must be less than <strong>@size MB.</strong><br/>Allowed file ' .
          'types: <strong>@ext.</strong>', array(
              '@size' => $upload_size,
              '@ext' => $extensions[0])),
      '#upload_location' => 'temporary://',
      '#upload_validators' => array(
        'file_validate_extensions' => $extensions,
         // Assume its specified in MB.
        'file_validate_size' => array($upload_size * 1024 * 1024),
      ),
    ),

/*    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Import datastreams'),
    ),
 */
  );
  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Next')
  );
  
  return $form;
}

function islandora_datastreams_io_review_import_file(array $form, array &$form_state, $do_push = FALSE) {
  $uploaded_file_name = '';
  if (isset($form_state['values']['uploaded_file_name']) && !empty($form_state['values']['uploaded_file_name'])) {
    $uploaded_file_name = $form_state['values']['uploaded_file_name'];
  } elseif (isset($_FILES['files']) && isset($_FILES["files"]["tmp_name"]['file'])) {
    if ($_FILES['files']['error']['file'] == UPLOAD_ERR_OK) {
      $uploaded_file_name = $_FILES["files"]["tmp_name"]['file'];
    }
  }
  $tmp_name = file_directory_temp() . '/datastream_io_archive_' . date('His_hms');
  if (isset($_SESSION['archive_file']) && $_SESSION['archive_file'] == '') {
    $_SESSION['archive_file'] = $tmp_name;
  }
  if ($uploaded_file_name && move_uploaded_file($uploaded_file_name, $tmp_name)) {
    $uploaded_file_name = $tmp_name;
  } else {
    if ($do_push && !$uploaded_file_name) {
      drupal_set_message('There was an error uploading the archive [' . $uploaded_file_name . ' :: ' . $tmp_name . '] file.');
    }
  }

  $extract_path = 'datastream_io_datastreams_extract_' . date('His_hms');

  $form['extract_path'] = array(
    '#title' => 'ZIP extracted to',
    '#type' => 'textfield',
    '#default_value' => $extract_path,
    '#attributes' => array('readonly' => 'readonly', 'disabled' => 'disabled'),
    '#suffix' => islandora_datastreams_io_analyze_uploaded_zipfile($extract_path, $uploaded_file_name, $do_push),
  );
  $form['uploaded_file_name'] = array(
    '#type' => 'hidden',
    '#default_value' => $uploaded_file_name,
  );
  if (!$do_push) {
    $form['back'] = array(
      '#type' => 'submit',
      '#value' => t('Back')
    );
    $form['next'] = array(
      '#type' => 'submit',
      '#value' => t('Finish')
    );
  }

  return $form;
}

function islandora_datastreams_io_import_form_submit(array $form, array &$form_state) {
  switch ($form_state['stage']) {
    case 'complete':
      $form_state['multistep_values'][$form_state['stage']] = $form_state['values'];
      if ($form_state['triggering_element']['#value'] != 'Back') {
        islandora_datastreams_io_complete_submit($form, $form_state);
        $form_state['complete'] = TRUE;
      }
     break;

    default:
      $form_state['multistep_values'][$form_state['stage']] = $form_state['values'];
      $form_state['new_stage'] = islandora_datastreams_io_move_to_next_stage($form, $form_state);
     break;
  }

  if (isset($form_state['complete'])) drupal_goto('complete-page');

  if ($form_state['triggering_element']['#value'] == 'Back') {
    $form_state['new_stage'] = islandora_datastreams_io_move_to_previous_stage($form, $form_state);
  }

  if (isset($form_state['multistep_values']['form_build_id'])) {
    $form_state['values']['form_build_id'] = $form_state['multistep_values']['form_build_id'];
  }
  $form_state['multistep_values']['form_build_id'] = $form_state['values']['form_build_id'];
  $form_state['stage'] = $form_state['new_stage'];
  $form_state['rebuild'] = TRUE;
}

function islandora_datastreams_io_move_to_next_stage($form, &$form_state) {
  switch ($form_state['stage']) {
    case 'import_pickfile':
      return 'review_import_file';
     break;

    case 'review_import_file':
      return 'ok_import_file';
     break;

    case 'ok_import_file':
      return 'complete';
     break;
  }
}

function islandora_datastreams_io_move_to_previous_stage($form, &$form_state) {
  switch ($form_state['stage']) {
    case 'review_import_file':
      return 'import_pickfile';
     break;

    case 'ok_import_file':
      return 'review_import_file';
     break;

    case 'complete':
      return 'ok_import_file';
     break;
  }
}

function islandora_datastreams_io_analyze_uploaded_zipfile($extract_path, $filename, $do_push) {
  global $user;
  module_load_include('inc', 'islandora_datastreams_io', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/utilities');

  $markup = '';
  $zip = new ZipArchive;

  if ($zip->open($filename) === TRUE) {
    $full_extract_path = file_directory_temp() . '/' . $extract_path;

    $zip->extractTo($full_extract_path);
    $zip->close();

    $schema_test_text_xml = variable_get('islandora_datastreams_io_schema_test_text_xml', TRUE);
    $mods_schema_version = ($schema_test_text_xml) ?
        variable_get('islandora_datastreams_io_mods_schema_xsd_version', 'MODS 3.7') : '';
    if ($mods_schema_version) {
      $mods_schema_versions = islandora_datastreams_io_get_mods_schema_versions();
      $mods_schema_uri = array_search($mods_schema_version, $mods_schema_versions);
    }
    else {
      $mods_schema_uri = '';
    }

    $raw_file_in_zip_count = $files_to_import = $skipped_namespace_restriction_count = 0;
    $markup .= '<h3>ZIP extraction ok.</h3>';
    $files = islandora_datastreams_io_get_exported_files($full_extract_path);
    if (count($files) > 0) {
      $cannot_import_markup = $files_markup = '';
      $dsid = '';
      $extra_files_in_zip = $file_count = $missing_islandora_objects = $extra_files_in_zip = array();
      foreach ($files as $file) {
        $ds_fileparts = islandora_datastreams_io_ds_file_parts($file);
        // Initialize the $file_count array if the file looks like it has an
        // Islandora object PID and DSID.
        if (array_key_exists('dsid', $ds_fileparts) && !array_key_exists($ds_fileparts['dsid'], $file_count)) {
          $file_count[$ds_fileparts['dsid']] = 0;
        }
      }
      foreach ($files as $file) {
        $ds_fileparts = islandora_datastreams_io_ds_file_parts($file);
        if (isset($ds_fileparts['pid']) && isset($ds_fileparts['namespace']) && isset($ds_fileparts['dsid'])) {
          if (!islandora_namespace_accessible($ds_fileparts['pid'])) {
            drupal_set_message('PID ' . $ds_fileparts['pid'] . ' is restricted and will be skipped.');
            @unlink($full_extract_path . '/' . $file);
            $skipped_namespace_restriction_count++;
            $files_markup .= '<span class="bad">PID = ' . $ds_fileparts['pid'] . 
                ', DSID = ' . $ds_fileparts['dsid'] . '</span> <i>skipped because of namespace restrictions</i><br>';
            $cannot_import_reasons[$file] = $ds_fileparts['pid'] . ' is skipped because of namespace restrictions';
          } else {
            // Check to see that this PID is an islandora_object.
            $test_islandora_object = islandora_object_load($ds_fileparts['pid']);
            if (is_object($test_islandora_object)) {
              // If the destination DSID is MODS, and the configuration is
              // set to perform a schema test on text/xml -- these could fail
              // for that reason as well.
              if ($ds_fileparts['dsid'] == 'MODS') {
                $import_mods_filename = $full_extract_path . '/' . $file;
                if ((!$schema_test_text_xml) || ($schema_test_text_xml && islandora_datastreams_io_mods_schema_success($import_mods_filename, $mods_schema_uri))) {
                  $files_markup .= 'PID = ' . $ds_fileparts['pid'] .
                      ', DSID = ' . $ds_fileparts['dsid'] . '<br>';
                  $files_to_import++;
                }
                else {
                  $cannot_import_reasons[$file] = $ds_fileparts['pid'] . ' failed a schema test';
                }
              }
            }
            else {
              $missing_islandora_objects[] = $ds_fileparts['pid'];
              $cannot_import_reasons[$file] = $ds_fileparts['pid'] . ' is not an Islandora object';
              $files_markup .= '<span class="bad">PID = ' . $ds_fileparts['pid'] . 
                  ', DSID = ' . $ds_fileparts['dsid'] . '</span> <i>skipped because Islandora object does not exist</i><br>';
            }
          }
          $file_count[$ds_fileparts['dsid']]++;
        }
        else {
          $extra_files_in_zip[] = $file;
          $cannot_import_reasons[$file] = 'does not appear to represent an Islandora object datastream file';
        }
        $raw_file_in_zip_count++;
        $dsid = (!$dsid && isset($ds_fileparts['dsid'])) ? $ds_fileparts['dsid'] : '';
      }
      if (!$do_push) {
        $markup .= '<h3>ZIP archive contains ' . number_format($raw_file_in_zip_count) . ' files</h3>';

        if (isset($cannot_import_reasons)) {
          if (count($cannot_import_reasons) > 0) {
            $cannot_import_markup = '<ul>';
            foreach ($cannot_import_reasons as $file => $reason) {
              $cannot_import_markup .= '<li>' . $reason . ' for file "' . $file . '"</li>';
            }
            $cannot_import_markup .= '</ul>';
          } 
        }

        if ($files_to_import) {
          $markup .= '<p>Clicking the [Finish] button would import ' . number_format($files_to_import) . ' Datastream/s.' .
            (($files_to_import <> $raw_file_in_zip_count) ? $cannot_import_markup : '') .
            '</p><pre>' . $files_markup . '</pre>';
        }
        else {
          $markup .= 'There are no datastreams to import.' . $cannot_import_markup;
        }

        islandora_datastreams_io_dump_temp_dir($full_extract_path);
      }
      else {
        // Determine mimetype of existing file and compare to old mimetype
        $islandora_object = islandora_object_load($ds_fileparts['pid']);
        $old_mimetype = $islandora_object[$ds_fileparts['dsid']]->mimeType;
        watchdog('test', 'Old mimetype is ' . $old_mimetype);
        $new_mimetype = islandora_datastreams_io_mimetype_of_file($file);
        watchdog('test', 'New mimetype is ' . $new_mimetype);
        $mimetype_command = ($old_mimetype != $new_mimetype ? " --datastreams_mimetype={$new_mimetype}" : "" ); 
        watchdog('test', 'Mimetype command is ' . $mimetype_command);

        // If datastream label is empty, use this value, else the previous label is used.
        $datastreams_label = 'Imported ' . $dsid;

        $this_domain = 'http://' . $_SERVER['HTTP_HOST'];
        $drush_command = 'drush islandora_datastream_crud_push_datastreams -u ' . $user->uid .
                         ' --datastreams_source_directory=' . $full_extract_path .
                         $mimetype_command .
                         ' -y ' .
                         ' --uri=' . $this_domain;

        $output = $return_var = array();
        exec($drush_command, $output, $return_var);
        if ($return_var == -1 || $return_var == 255) {
          drupal_set_message('There was an error calling the drush command : ' . $drush_command);
        }
        else {
          $markup .= '<h3>' . $dsid . ' Datastreams Pushed: ' . number_format($files_to_import) . '</h3><pre>' . $files_markup . '</pre>';
        }
        islandora_datastreams_io_dump_temp_dir($full_extract_path);

        if (isset($_SESSION['archive_file']) && $_SESSION['archive_file']) {
          @unlink($_SESSION['archive_file']);
        }
      }
      // If the dsid is MODS - and there is a transform to DC configured, we
      // need to trigger this here.
      if ($dsid == 'MODS') {

      }
    }
    else {
      $markup .= 'There were no files in the ZIP file.';
    }

  } else {
    $markup .= '<h3>ZIP extraction failed</h3>';
  }
  return $markup;
}
