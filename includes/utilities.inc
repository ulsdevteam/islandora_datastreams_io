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
