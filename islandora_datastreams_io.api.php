<?php

/**
 * @file
 * This file documents all available hook functions to manipulate data.
 */

/**
 * Example hook function to handle the MODS schema check.
 *
 * Analyze the $mods_filename file as needed and return TRUE / FALSE based
 * on whether or not the file should be imported.
 *
 * @param string $mods_filename
 *   The MODS file location - saved as a temporary file.  There is no need
 *   to delete this file after the schema check because the import process
 *   will clean up the unzipped import file (see the function
 *   "islandora_datastreams_io_analyze_uploaded_zipfile" in
 *   includes/import.form.inc).
 *
 * @return boolean
 *   Success / Failure of the MODS file check.
 */
function hook_mods_schema_check($mods_filename) {
  // For example, if you did not want to import any MODS that contained the
  // word "foobar", code it like this:
  $mods_contents = file_get_contents($mods_filename);
  return (strstr($mods_contents, "foobar") <> '');
}
