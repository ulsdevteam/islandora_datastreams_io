<?php

function islandora_datastreams_io_import_form(array $form, array &$form_state) {
  module_load_include('inc', 'upitt_workflow', 'includes/utilities');

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
  dpm($form_state);
  $markup_import = '<p>' . t('This will allow a specific datastream from a given set of objects to be downloaded. ' .
          'A set of objects can be defined by a Solr query, by collection membership, by content ' .
          'model, or by a list of PID values.') . '</p>';

  // Prepare a couple variables for use with file upload options:
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $extensions = array('zip');

  $form['importform'] = array(
    'form_description' => array('#markup' => $markup_import),
//    'breadcrumb' => array('#markup' => upitt_workflow_get_breadcrumb_path()),

    'file' => array(
      '#type' => 'file',
      '#title' => t('File'),
      '#description' => t('Select ZIP file of datastream files.<br/>Files must be less than <strong>@size MB.</strong><br/>Allowed file types: <strong>@ext.</strong>', array('@size' => $upload_size, '@ext' => $extensions[0])),
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
  if ($uploaded_file_name && move_uploaded_file($uploaded_file_name, $tmp_name)) {
    $uploaded_file_name = $tmp_name;
  } else {
    drupal_set_message('There was an error uploading the archive file.');
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
    '#type' => 'textfield',
    '#default_value' => $uploaded_file_name,
  );
  $form['back'] = array(
    '#type' => 'submit',
    '#value' => t('Back')
  );
  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Finish')
  );

  return $form;
}

function islandora_datastreams_io_import_form_submit(array $form, array &$form_state) {
  dpm($form_state);
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

  $markup = '';
  $zip = new ZipArchive;

  if ($zip->open($filename) === TRUE) {
    $full_extract_path = file_directory_temp() . '/' . $extract_path;

    $zip->extractTo($full_extract_path);
    $zip->close();

    $file_count = 0;
    $markup .= '<h3>ZIP extraction ok.</h3>';
    $files = islandora_datastreams_io_get_exported_files($full_extract_path);
    if (count($files) > 0) {
      $files_markup = '';
      foreach ($files as $file) {
        
        @list($namespace, $restofthepid, $dsid) = explode('_', $file);
        $this_pid = $namespace . ':' . $restofthepid;
        $filename_pid_removed = ltrim(str_replace($namespace . '_' . $restofthepid, '', $file), '_');
        $path_parts = pathinfo($filename_pid_removed);
        $dsid = $path_parts['filename'];

        $files_markup .= 'PID = ' . $this_pid . ' DSID = ' . $dsid . '<br>';
        $file_count++;
      }
      if (!$do_push) {
        $markup .= '<h3>ZIP archive contains ' . number_format($file_count) . ' '. $dsid . ' Datastream/s</h3><pre>' . $files_markup . '</pre>';
      }
      else {
        $this_domain = 'http://' . $_SERVER['HTTP_HOST'];
        $drush_command = 'drush islandora_datastream_crud_push_datastreams -u ' . $user->uid .
                         ' --datastreams_source_directory=' . $full_extract_path .
                         ' --silent=1 ' .
                         ' --uri=' . $this_domain;

        $output = $return_var = array();
        exec($drush_command, $output, $return_var);
        if ($return_var == -1 || $return_var == 255) {
          drupal_set_message('There was an error calling the drush command : ' . $drush_command);
        }
        else {
          $markup .= '<h3>' . $dsid . ' Datastreams Pushed: ' . number_format($file_count) . '</h3><pre>' . $files_markup . '</pre>';
        }
      }
    }
    else {
      $markup .= 'There were no datastream files in the ZIP file.';
    }

  } else {
    $markup .= '<h3>ZIP extraction failed</h3>';
  }
  return $markup;
}