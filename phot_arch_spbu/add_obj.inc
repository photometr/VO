<?php

/**
 * @file
 * Demonstrations of AJAX with graceful degradation.
 */

/**
 * @defgroup ajax Example: AJAX Graceful Degradation
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
 * This example shows a button to "add more" - add another textfield, and
 * the corresponding "remove" button.
 *
 * It works equivalently with javascript or not, and does the same basic steps
 * either way.
 *
 * The basic idea is that we build the form based on the setting of
 * $form_state['num_names']. The custom submit functions for the "add-one"
 * and "remove-one" buttons increment and decrement $form_state['num_names']
 * and then force a rebuild of the form.
 *
 * The $no_js_use argument is simply for demonstration: When set, it prevents
 * '#ajax' from being set, thus making the example behave as if javascript
 * were disabled in the browser.
 */
function phot_arch_spbu_form_add_object($form, &$form_state, $no_js_use = FALSE) {
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

  // Because we have many fields with the same values, we have to set
  // #tree to be able to access them.
  $form['#tree'] = TRUE;
  $form['names_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Добавить объекты в базу данных'),
    // Set up the wrapper so that AJAX will be able to replace the fieldset.
    '#prefix' => '<div id="names-fieldset-wrapper">',
    '#suffix' => '</div>',
  );

  // Build the fieldset with the proper number of names. We'll use
  // $form_state['num_names'] to determine the number of textfields to build.
  if (empty($form_state['num_names'])) {
    $form_state['num_names'] = 1;
  }
  for ($i = 0; $i < $form_state['num_names']; $i++) {
    $form['names_fieldset']['name'][$i] = array(
      '#type' => 'textfield',
      '#title' => t('Имя блазара'),
    );
  }
  $form['names_fieldset']['add_name'] = array(
    '#type' => 'submit',
    '#value' => t('Добавить ещё'),
    '#submit' => array('phot_arch_spbu_form_add_object_add_one'),
    // See the examples in ajax_example.module for more details on the
    // properties of #ajax.
    '#ajax' => array(
      'callback' => 'phot_arch_spbu_form_add_object_callback',
      'wrapper' => 'names-fieldset-wrapper',
    ),
  );
  if ($form_state['num_names'] > 1) {
    $form['names_fieldset']['remove_name'] = array(
      '#type' => 'submit',
      '#value' => t('Удалить последний'),
      '#submit' => array('phot_arch_spbu_form_add_object_remove_one'),
      '#ajax' => array(
        'callback' => 'phot_arch_spbu_form_add_object_callback',
        'wrapper' => 'names-fieldset-wrapper',
      ),
    );
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Записать'),
  );

  // This simply allows us to demonstrate no-javascript use without
  // actually turning off javascript in the browser. Removing the #ajax
  // element turns off AJAX behaviors on that element and as a result
  // ajax.js doesn't get loaded.
  // For demonstration only! You don't need this.
  if ($no_js_use) {
    // Remove the #ajax from the above, so ajax.js won't be loaded.
    if (!empty($form['names_fieldset']['remove_name']['#ajax'])) {
      unset($form['names_fieldset']['remove_name']['#ajax']);
    }
    unset($form['names_fieldset']['add_name']['#ajax']);
  }

  return $form;
}

/**
 * Custom validation form for the 'get data' page.
 *
 * @ingroup VO
 */
function phot_arch_spbu_form_add_object_validate($form, &$form_state) {
  for ($i = 0; $i < $form_state['num_names']; $i++) {
    if (strlen($form_state['values']['names_fieldset']['name'][$i]) < 1){
      form_set_error('names_fieldset', t('Object name must not be empty'));
    }
  }
}

/**
 * Callback for both ajax-enabled buttons.
 *
 * Selects and returns the fieldset with the names in it.
 */
function phot_arch_spbu_form_add_object_callback($form, $form_state) {
  return $form['names_fieldset'];
}

/**
 * Submit handler for the "add-one-more" button.
 *
 * Increments the max counter and causes a rebuild.
 */
function phot_arch_spbu_form_add_object_add_one($form, &$form_state) {
  $form_state['num_names']++;
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for the "remove one" button.
 *
 * Decrements the max counter and causes a form rebuild.
 */
function phot_arch_spbu_form_add_object_remove_one($form, &$form_state) {
  if ($form_state['num_names'] > 1) {
    $form_state['num_names']--;
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Write new names to DB on submit.
 */
function phot_arch_spbu_form_add_object_write_db($form, $form_state) {
  $object_names = $form_state['values']['names_fieldset']['name'];
  $trans_value = db_transaction();

  try {
    foreach ($object_names as $object_name) {
      $data_entries[] = array(
        'name'  => $object_name,
      );
    }
    $query = db_insert('phot_arch_spbu_objects')->fields(array(
                                          'name',
                                          ));
    foreach ($data_entries as $data_entry) {
      $query->values($data_entry);
    }
    $query->execute();
  }
  catch (Exception $e) {
    $trans_value->rollback();
    drupal_set_message(t('db_insert failed. Message = %message, query= %query',
      array('%message' => $e->getMessage(), '%query' => $e->query_string)), 'error');
  }
}

/**
 * Final submit handler.
 *
 * Reports what values were finally set.
 */
function phot_arch_spbu_form_add_object_submit($form, &$form_state) {
  phot_arch_spbu_form_add_object_write_db($form, $form_state);
  $output = t('Эти объекты были добавлены в базу: @names',
    array('@names' => implode(', ', $form_state['values']['names_fieldset']['name'])) );
  drupal_set_message($output);
}
/**
 * @} End of "defgroup ajax_degradation_example".
 */
