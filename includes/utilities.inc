<?php

function islandora_datastreams_io_get_exported_files($dir) {
  $files = array();
  $cdir = scandir($dir);
  foreach ($cdir as $key => $value) {
    if (!in_array($value,array(".",".."))) {
      $files[] = $value;
    }
  }
  return $files;
}

function islandora_datastreams_io_ds_file_parts($filename) {
  $last_ = strrchr($filename, "_");
  if (strlen($last_)) {
    $dsid_ext = substr($filename, (-strlen($last_) + 1));
    $ns_pid = substr($filename, 0, (strlen($filename) - strlen($last_)));

    @list($namespace, $pid) = explode("_", $filename, 2);
    @list($dsid, $ext) = explode(".", $dsid_ext);
    return array('namespace' => $namespace, 'pid' => $namespace . ':' . $pid, 'dsid' => $dsid, 'extension' => $ext);
  }
  else {
    return array();
  }
}

/**
 * Delete the temporary uncompressed folder from zip file upload.
 */
function islandora_datastreams_io_dump_temp_dir($full_extract_path) {
  $files = glob(rtrim($full_extract_path, '/') . '/*');
  foreach($files as $file){
    if(is_file($file))
      unlink($file);
  }
  rmdir($full_extract_path);
}
