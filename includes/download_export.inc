<?php

function islandora_datastreams_download_export($base64_encoded_zip_name) {
  $zip_name = base64_decode($base64_encoded_zip_name);

  header('Content-Type: application/zip');
  header('Content-disposition: attachment; filename='.basename($zip_name));
  header('Content-Transfer-Encoding: Binary');
  header('Content-Length: ' . filesize($zip_name));
  readfile($zip_name);
  
  unlink($zip_name);
}

