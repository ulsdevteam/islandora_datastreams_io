/**
 * @file
 * Javascript file for islandora_datastreams_io
 */

(function ($) {

    $(document).ready(function() {
        $('#edit-pids-fetch-method-0').change(function() {
            hideall_but(0);
        });
        $('#edit-pids-fetch-method-1').change(function() {
            hideall_but(1);
        });
        $('#edit-pids-fetch-method-2').change(function() {
            hideall_but(2);
        });
        $('#edit-pids-fetch-method-3').change(function() {
            hideall_but(3);
        });
        hideall_but(0);
    });
  
    function hideall_but(i) {
        if (i < 1) {
            $('.form-item-solr-query').show();
            $('.form-item-batch-csv').hide();
            $('.form-item-collection').hide();
            $('.form-item-model').hide();
        }
        if (i == 1) {
            $('.form-item-solr-query').hide();
            $('.form-item-batch-csv').show();
            $('.form-item-collection').hide();
            $('.form-item-model').hide();
        }
        if (i == 2) {
            $('.form-item-solr-query').hide();
            $('.form-item-batch-csv').hide();
            $('.form-item-collection').show();
            $('.form-item-model').hide();
        }
        if (i == 3) {
            $('.form-item-solr-query').hide();
            $('.form-item-batch-csv').hide();
            $('.form-item-collection').hide();
            $('.form-item-model').show();
        }
    };
  
})(jQuery);

