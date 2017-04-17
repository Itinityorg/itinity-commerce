<?php
/**
* HOOK catalog_sql_info()
* Informs catalog module about sortable/filterable fields available for selected entities type/bundle
* Return associative array of avalable methods and fields
*/
function hook_catalog_sql_info($catalog_object) {
  if (shop_get_info('entity_types', $catalog_object->entity_type, $catalog_object->bundle)) {
    return array(
      'orders' => array( // Settings in this sub-array will be used for ordering entities
        'price' => array( // Internal order method name for catalog, must be not equal to name of entity (extra)field. If it not equal to field name, you must specify 'label property for displaying it in order links.
          'description' => t('By price'), // Description for admin for default order method setting
          'label' => t('Product price'), // Optional. Can be required when needs overwrite (extra)field label or if order method is not equal to (extra)field name, however it must be displayed in sorting links
          'default' => '',// Optional boolean. This order method will forced enabled when currently enabled order methods array is empty.
          'by' => 'si.price', // Main property. Name of sortable field, that will passed in 'ORDER BY ...' statement. For initially reverse sorting use 'si.price DESC'.
          'isnull' => FALSE, // Optional boolean. Inform catalog about that 'si.price' field never can be NULL or ''. This used for smart sorting (NULL fields always placed to end where 'isnull' property is empty)
          'commands' => array( // Other commands which will be passed into db_select() object. This can be needed where name of sql field is not found in base table of entity type.
            'innerJoin' => array( // -->> $query->innerJoin(..)
              array( // -->> $query->innerJoin('shop_index', 'si', 'si.entity_type = :entity_type AND si.entity_id = base.nid', array(':entity_type' => $entity_type,),)
                'shop_index', 
                'si',
                'si.entity_type = :entity_type AND si.entity_id = base.nid',
                array(':entity_type' => $catalog_object->entity_type,),              
              ),
            ),
          ),
        ),
        'random' => array(
          'description' => t('Random'),
          'label' => t('Random'),
          'by' => 'random_field', // 'random_field' is added as expression by core in $query->orderRandom() function
          'suppress pager' => TRUE, // Pager will not used if this ordering method is enabled.
          'commands' => array(
            'orderRandom' => array(
              array(),
            ),
          ),
        ),
      ),
      'filters' => array( // Settings in this sub-array will be used for entities filtering
        'qty' => array( // Internal order method name, must be not equal to name of entity field
          'description' => t('In stock'), // Description for admin
          'label' => t('Product price'), // Optional. Can be required when needs overwrite field label or order method not contains field_name, however it must be displayed in exposed filters elements
          'default' => '',// Optional. Default state of this filter.
          'where' => 'si.qty > 0 OR si.qty IS NULL', // Optional. Condition statement which will be used as additional filter in db_select() object. Also you can place this condition in next 'commands' sub array.
          'commands' => array( // Other commands which will be passed into db_select() object. This can be needed where name of sql field is not found in base table of entity type.
            'innerJoin' => array( // -->> $query->innerJoin(..)
              array( // -->> $query->innerJoin('shop_index', 'si', 'si.entity_type = :entity_type AND si.entity_id = base.nid', array(':entity_type' => $entity_type,),)
                'shop_index',
                'si',
                'si.entity_type = :entity_type AND si.entity_id = base.nid',
                array(':entity_type' => $catalog_object->entity_type,),              
              ),
            ),
          ),
        ),
      ),
    );
  }
}