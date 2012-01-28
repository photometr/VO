<?php

/**
 * @file
 * AJAX with graceful degradation.
 */

/**
 * @defgroup ajax
 * @ingroup VO
 * @{
 * These examples show AJAX with graceful degradation when Javascript is not
 * available.
 *
 * In each of these the key idea is that the form is rebuilt different ways
 * depending on form input. In order to accomplish that, the formbuilder function
 * is in charge of almost all logic.
 */

/**
 * A form with a dropdown whose options are dependent on a
 * choice made in a previous dropdown.
 *
 * On changing the first dropdown, the options in the second
 * are updated. Gracefully degrades if no javascript.
 *
 * A bit of CSS and javascript is required. The CSS hides the "add more" button
 * if javascript is not enabled. The Javascript snippet is really only used
 * to enable us to present the form in degraded mode without forcing the user
 * to turn off Javascript.  Both of these are loaded by using the
 * #attached FAPI property, so it is a good example of how to use that.
 *
 * The extra argument $no_js_use is here only to allow presentation of this
 * form as if Javascript were not enabled. ajax_example_menu() provides two
 * ways to call this form, one normal ($no_js_use = FALSE) and one simulating
 * Javascript disabled ($no_js_use = TRUE).
 */
function phot_arch_form_get($form, &$form_state) {
  // Get the list of options to populate the first dropdown.
  $blazars_names = blazars_list_load();

  // If we have a value for the first dropdown from $form_state['values'] we use
  // this both as the default value for the first dropdown and also as a
  // parameter to pass to the function that retrieves the options for the
  // second dropdown.
  $selected = isset($form_state['values']['name']) ? $form_state['values']['name'] : key($blazars_names);

  $form = array(
    '#prefix' => '<div id="main-replace">',
    '#suffix' => '</div>',
  );

  $form['name_fieldset'] = array(
    '#type' => 'fieldset',
  );
  $form['name_fieldset']['name'] = array(
    '#type' => 'select',
    '#title' => 'Имя блазара:',
    '#options' => $blazars_names,
    '#attributes' => array('class' => array('enabled-for-ajax')),

    // The '#ajax' property allows us to bind a callback to the server whenever this
    // form element changes. See ajax_example_autocheckboxes and
    // ajax_example_dependent_dropdown in ajax_example.module for more details.
    '#ajax' => array(
      'callback' => 'ajax_name_dropdown_callback',
      'wrapper' => 'dropdown-second-replace',//dropdown-second-replace
    ),
  );

  $form['band_fieldset'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<div id="dropdown-second-replace">',
    '#suffix' => '</div>',
  );
  $form['band_fieldset']['band'] = array(
    '#type' => 'select',
    '#title' => 'Полоса',
    '#attributes' => array('class' => array('enabled-for-ajax')),
    // When the form is rebuilt during processing (either AJAX or multistep),
    // the $selected variable will now have the new value and so the options
    // will change.
    '#options' => bands_list_load($selected),
    '#default_value' => isset($form_state['values']['band']) ? $form_state['values']['band'] : '',
    '#ajax' => array(
      'callback' => 'ajax_band_dropdown_callback',
      'wrapper' => 'dropdown-second-replace',
    ),
  );
  $form['band_fieldset']['JDstart'] = array(
    '#type'  => 'textfield',
    '#title' => t('От JD:'),
    '#size'  => 10,
    '#attributes' => array('class' => array('enabled-for-ajax')),
    '#value' => isset($form_state['values']['band']) ? $form_state['values']['band'] : '',
    '#default_value' => '',
  );
  $form['band_fieldset']['JDend'] = array(
    '#type'  => 'textfield',
    '#title' => t('До JD:'),
    '#size'  => 10,
  );
  $form['band_fieldset']['ASCII'] = array(
    '#type' => 'submit',
    '#value' => t('Скачать ASCII'),
    '#submit' => array('phot_arch_form_get_ASCII_submit'),
    // This class allows attached js file to override the disabled attribute,
    // since it's not necessary in ajax-enabled form.
    '#attributes' => array('class' => array('enabled-for-ajax')),
  );
  $form['band_fieldset']['VOT'] = array(
    '#type' => 'submit',
    '#value' => t('Скачать VOT'),
    '#submit' => array('phot_arch_form_get_VOT_submit'),
    '#attributes' => array('class' => array('enabled-for-ajax')),
  );
  $form['band_fieldset']['bibtex'] = array(
    '#type' => 'submit',
    '#value' => t('Скачать Bibtex'),
    '#submit' => array('phot_arch_form_get_bibtex_submit'),
    '#attributes' => array('class' => array('enabled-for-ajax')),
  );

  // Disable band if a selection has not been made on dropdown_first.
  if (empty($form_state['values']['name'])) {
    $form['band_fieldset']['band']['#disabled'] = TRUE;
    $form['band_fieldset']['band']['#description'] = t('You must make your choice on the first dropdown before changing this second one.');
    $form['band_fieldset']['JDstart']['#disabled'] = TRUE;
    $form['band_fieldset']['JDend']['#disabled'] = TRUE;
    $form['band_fieldset']['ASCII']['#disabled'] = TRUE;
    $form['band_fieldset']['VOT']['#disabled'] = TRUE;
    $form['band_fieldset']['bibtex']['#disabled'] = TRUE;
  }

  return $form;
}


/**
 * Selects just the second dropdown to be returned for re-rendering.
 *
 * @return
 *   Renderable array (the second dropdown).
 */
function ajax_name_dropdown_callback($form, $form_state) {
  $form['band_fieldset']['JDstart']['#value'] = NULL;
  return $form['band_fieldset'];//['band'];
}

/**
 * Selects just the second dropdown to be returned for re-rendering.
 *
 * @return
 *   Renderable array (the second dropdown).
 */
function ajax_band_dropdown_callback($form, $form_state) {
  return $form['band_fieldset'];
}


/**
 * @} End of "defgroup ajax".
 */
