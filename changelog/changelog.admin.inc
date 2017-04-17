<?php

/**
* Callback for /admin/reports/changelog
*/
function changelog_events_overview_form($form, &$form_state) {
	$query = db_select('changelog', 'ch')->fields('ch', array('changelog_id', 'uid', 'entity_type', 'entity_id'));
	$header = array(
		'_created' => array(
			'data' => t('Date'),
			'field' => 'ch.created',
			'sort' => 'desc',
		),
		'_message' => array(
			'data' => t('Message'),
			'field' => 'ch.message',
		),
		'_uid' => array(
			'data' => t('User'),
		),
		'_entity' => array(
			'data' => t('Title'),
		),
	);
	$query = $query->extend('PagerDefault')->limit(50);
	$query = $query->extend('TableSort');
	$query->orderByHeader($header);
	foreach ($query->execute() as $r) {
		$ids[$r->changelog_id] = $r;
	}

	if (!empty($ids)) {
		$rows = array();
		foreach (changelog_event_load_multiple(array_keys($ids)) as $event) {
			$entity_uri = $entity_label = $comment_link = NULL;
			if ($entities = entity_load($event->entity_type, array($event->entity_id))) {
				$entity = reset($entities);
				$entity_uri = entity_uri($event->entity_type, $entity);
				if ($event->entity_type === 'node' && $event->comment_cid && module_exists('comment')) {
					$comment_link['path'] = $entity_uri['path'];
					$comment_link['page'] = comment_get_display_page($event->comment_cid, $entity->type);
				}
				$entity_label = entity_label($event->entity_type, $entity);
				
			}
			$message = t($event->message, $event->message_params);
			if ($comment_link) {
				$message = l($message, $comment_link['path'], array('html' => TRUE, 'fragment' => 'comment-' . $event->comment_cid, 'query' => $comment_link['page'] ? array('page' => $comment_link['page']) : array()));
			}
			$message .= ($event->raw_text ? '<br /><small><em>' . filter_xss($event->raw_text, array('a', 'em', 'strong', 'span', 'br',)) . '</em></small>' : '');
			$rows[] = array(
				'_created' => format_date($event->created, 'short'),
				'_message' => $message,
				'_uid' => theme('username', array('account' => user_load($event->uid))),
				'_entity' => $entity_uri ? l($entity_label, $entity_uri['path']) : '',
			);
		}
		$form['changelog'] = array(
			'#theme' => 'table',
			'#header' => $header,
			'#rows' => $rows,
		);
		$form['pager']['#theme'] = 'pager';
	}
	else {
		$form[]['#markup'] = t('Events not found');
	}

	return $form;
}














