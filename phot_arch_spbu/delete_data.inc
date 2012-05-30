<?php

/**
 * 'Delete entries' form
 * Allows to delete list of dates from the DB
 */
function phot_arch_spbu_form_delete($form, &$form_state) {
  //check if user allowed to add to the database
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
  $form = array();

  $form = array(
    '#prefix' => '<div id="updateform">',
    '#suffix' => '</div>',
  );
  $form['name'] = array(
    '#type' => 'select',
    '#options' => blazars_list_load(),
    '#title' => t('Имя блазара:'),
  );
  $form['band'] = array(
    '#type'  => 'select',
    '#options' => bands_list_load_all(),
    '#title' => t('Фотометрическая полоса:'),
  );
  $form['data'] = array(
    '#type'  => 'textarea',
    '#title' => 'Даты для удаления:',
    '#size'  => 15,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Добавить'),
  );

  return $form;
}


/**
 * Submit handler for 'delete entries' form.
 */
function phot_arch_spbu_form_delete_submit($form, &$form_state) {
  $entry = array(
    'blazar_name' => $form_state['values']['name'],
    'band'        => $form_state['values']['band'],
    'data'        => $form_state['values']['data']
  );
  $return = phot_arch_spbu_entries_delete($entry);
  if ($return) {
    drupal_set_message(t("Данные успешно удалены"));
  }
}

/**
 * Handler for 'delete entries' requests
 */
function phot_arch_spbu_entries_delete($entry) {
  global $user;
  //check if user allowed to add the database
  if (! in_array("phot_db_maintainer", $user->roles)) {
    drupal_set_message(t('ask Admin to be added to phot_db_maintainer role'), 'error');
    return NULL;
  }
  $band_id = $entry['band'];
  if ($band_id == 8){//FIXME dangerous written in phot_arch_spbu.install
    $query = phot_arch_spbu_entries_delete_polar($entry);
  }
  else{
    $query = phot_arch_spbu_entries_delete_phot($entry);
  }
  return $query;
}

/**
 * Delete polarimetry entries from the database.
 *
 */
function phot_arch_spbu_entries_delete_polar($entry) {
  global $user;

  $blazar_id = $entry['blazar_name'];

  //parsing data
  $data_arr = preg_split("/((?<!\\\|\r)\n)|((?<!\\\)\r\n)/", $entry['data']);
  foreach ($data_arr as $value) {
    $value = trim ($value);
    if ($value == "") continue;
    $one_line  = preg_split("/[\s,]+/", $value);

    if (!(is_numeric($one_line[0]))) {
      drupal_set_message(t('It seems that you give not numeric data.'), 'error');
      return NULL;
    }
    $JD[] = floatval($one_line[0]);
  }

  $trans_value = db_transaction();
  try {
    foreach ($JD as $date) {
      $result = db_delete('phot_arch_spbu_polar')
                   ->condition('blazar_id', $blazar_id)
                   ->condition('jd', $date)
                   ->execute();
    }
    $result = True;
  }
  catch (Exception $e) {
    $trans_value->rollback();
    drupal_set_message(t('db_delete failed. Message = %message, query= %query',
      array('%message' => $e->getMessage(), '%query' => $e->query_string)), 'error');
    $result = False;
  }
  return $result;
}

/**
 * Delete photometry entries from the database.
 *
 */
function phot_arch_spbu_entries_delete_phot($entry) {
  global $user;

  $blazar_id = $entry['blazar_name'];
  $band_id = $entry['band'];
  //parsing data
  $data_arr = preg_split("/((?<!\\\|\r)\n)|((?<!\\\)\r\n)/", $entry['data']);
  foreach ($data_arr as $value) {
    $value = trim ($value);
    if ($value == "") continue;
    $one_line  = preg_split("/[\s,]+/", $value);

    if (!(is_numeric($one_line[0]))) {
      drupal_set_message(t('It seems that you give not numeric data.'), 'error');
      return NULL;
    }
    $JD[] = floatval($one_line[0]);
  }

  $trans_value = db_transaction();
  try {
    foreach ($JD as $date) {
      $result = db_delete('phot_arch_spbu_data')
                   ->condition('blazar_id', $blazar_id)
                   ->condition('band_id', $band_id)
                   ->condition('jd', $date)
                   ->execute();
    }
    $result = True;
  }
  catch (Exception $e) {
    $trans_value->rollback();
    drupal_set_message(t('db_delete failed. Message = %message, query= %query',
      array('%message' => $e->getMessage(), '%query' => $e->query_string)), 'error');
    $result = False;
  }
  return $result;
}
