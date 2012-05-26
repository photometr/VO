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
function phot_arch_spbu_form_get($form, &$form_state) {
  //check if user allowed to add the database
  global $user;
  if (! in_array("phot_db_maintainer", $user->roles)) {
    $error_text = '<font size="3" color="red">Только зарегистрированные пользователи имеют доступ к данным!</font>';
    $form['access_fieldset'] = array(
      '#type' => 'fieldset',
      '#title' => t($error_text),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    return $form;
  }
  //drupal_add_js('misc/jscharts/jquery.min.js');
  ////drupal_add_js('misc/jscharts/plot.js');
  //drupal_add_js('misc/jscharts/jquery.jqplot.min.js');
  //drupal_add_js('misc/jscharts/jqplot.dateAxisRenderer.min.js');
  //drupal_add_js('misc/jscharts/jqplot.cursor.min.js');
  //drupal_add_js('misc/jscharts/jqplot.highlighter.min.js');
  //drupal_add_js('misc/jscharts/jquery.jqplot.min.css');
  drupal_add_js('misc/dygraphs/dygraph-dev.js');
  //drupal_add_js('misc/dygraphs/data.js');
  //drupal_add_js('misc/dygraphs/plot_js.js');
  // Get the list of options to populate the first dropdown.
  $blazars_names = blazars_list_load();

  // If we have a value for the first dropdown from $form_state['values'] we use
  // this both as the default value for the first dropdown and also as a
  // parameter to pass to the function that retrieves the options for the
  // second dropdown.
  $selected = isset($form_state['values']['name']) ? $form_state['values']['name'] : key($blazars_names);

  //$form = array(
  //  '#prefix' => '<div id="main-replace">',
  //  '#suffix' => '</div>',
  //);

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
      'wrapper' => 'JD-replace',
    ),
  );
  $form['JD_fieldset'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<div id="JD-replace">',
    '#suffix' => '</div>',
  );
  $form['JD_fieldset']['JDstart'] = array(
    '#type'  => 'textfield',
    '#title' => t('От JD:'),
    '#size'  => 10,
    '#attributes' => array('class' => array('enabled-for-ajax')),
    //'#value' => isset($form_state['values']['band']) ? $form_state['values']['band'] : '',
    '#default_value' => '',
  );
  $form['JD_fieldset']['JDend'] = array(
    '#type'  => 'textfield',
    '#title' => t('До JD:'),
    '#size'  => 10,
  );
  $form['JD_fieldset']['plot'] = array(
    '#prefix' => '<div id="plot_div">',
    '#suffix' => '</div>',
    //'#markup' => get_plot_code($form, $form_state),
    //'#markup' => '<div id="chart3" style="margin-top:20px; margin-left:20px; width:500px; height:300px;"></div>',
    '#attributes' => array('class' => array('enabled-for-ajax')),
  );
  $form['JD_fieldset']['ASCII'] = array(
    '#type' => 'submit',
    '#value' => t('Скачать ASCII'),
    '#submit' => array('phot_arch_spbu_form_get_ASCII_submit'),
    // This class allows attached js file to override the disabled attribute,
    // since it's not necessary in ajax-enabled form.
    '#attributes' => array('class' => array('enabled-for-ajax')),
  );
  //$form['JD_fieldset']['VOT'] = array(
  //  '#type' => 'submit',
  //  '#value' => t('Скачать VOT'),
  //  '#submit' => array('phot_arch_spbu_form_get_VOT_submit'),
  //  '#attributes' => array('class' => array('enabled-for-ajax')),
  //);

  // Disable band if a selection has not been made on dropdown_first.
  if (empty($form_state['values']['name'])) {
    $form['band_fieldset']['band']['#disabled'] = TRUE;
    $form['band_fieldset']['band']['#description'] = t('Сначала выберите объект.');
    $form['JD_fieldset']['JDstart']['#disabled'] = TRUE;
    $form['JD_fieldset']['JDend']['#disabled'] = TRUE;
    $form['JD_fieldset']['ASCII']['#disabled'] = TRUE;
    $form['JD_fieldset']['VOT']['#disabled'] = TRUE;
    $form['JD_fieldset']['bibtex']['#disabled'] = TRUE;
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
  //$form['band_fieldset']['JDstart']['#value'] = NULL;
  //$form['band_fieldset']['plot']['#markup'] = get_plot_code();
  return $form['band_fieldset'];//['band'];
}

/**
 * Selects just the second dropdown to be returned for re-rendering.
 *
 * @return
 *   Renderable array (the second dropdown).
 */
function ajax_band_dropdown_callback($form, $form_state) {
  $form['JD_fieldset']['plot']['#markup'] = get_plot_code($form, $form_state);
  return $form['JD_fieldset'];
//$commands = array();
//$commands[] = ajax_command_replace('#plot_div', get_plot_code());
//return array('#type' => 'ajax', '#commands' => $commands);
}


/**
 * @} End of "defgroup ajax".
 */
