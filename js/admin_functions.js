/**
 * @file
 * Javascript file for islandora_datastreams_io
 * 
 */

(function ($) {

    $(document).ready(function() {
        $('#edit-schema-test-text-xml').change(function() {
            var i = $('input[name=schema_test_text_xml]:checked').val();
            if (i == undefined) {
                // disable the MODS schema input box
                
            }
            else {
                // enable the MODS schema input box
                
            }
        });
    });
  
})(jQuery);
