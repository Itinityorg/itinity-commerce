(function ($) {
	"use strict";
	Drupal.behaviors.address_element = {
		/**
		 * Attaches the behavior.
		 */
		attach: function (context, settings) {
			if (settings.addressElements) {
        console.log(settings.addressElements);
				$.each(settings.addressElements, function (sourceId, options) {
					$('#' + sourceId, context).once('address', function () {
						var id = '#' + sourceId;

						// Create DOM id's for manipulating
						//address elements
						/*
            var $countryElement = $(id + '-ajax-address-wrapper ' + id + '-country-wrapper input');
						var $stateElement = $(id + '-ajax-address-wrapper ' + id + '-state-wrapper input');
						var $cityElement = $(id + '-ajax-address-wrapper ' + id + '-city-wrapper input');
						var $streetElement = $(id + '-ajax-address-wrapper ' + id + '-street-wrapper input');
						var $buildingElement = $(id + '-ajax-address-wrapper ' + id + '-building-wrapper input');
            */  
						//gmap elements

						var $lat = $(id + ' input.form-text.latitude');
						var $lng = $(id + ' input.form-text.longitude');

						// Create default marker positions
						var Lat = $lat.val();
						var Lng = $lng.val();
            var $map_div = $('.gmap-element-container-map', this);
            $map_div.hide(); // спрячем чтоб отрисовать и отцентрировать 

            var myPlacemark;
            
            ymaps.ready(function () {

              var map = new ymaps.Map( $map_div[0], {
                center: [Lat, Lng],
                zoom: 16,
                controls: ['zoomControl', 'typeSelector']
              });
              map.events.add('click', function (e) {
                setPlacemark(e.get('coords'));
              });
          
              if (!Lat && !Lng) {
                // определим местоположение клиента и только после этого загрузим карту 
                ymaps.geolocation.get({
                  provider: 'yandex',
                  mapStateAutoApply: true
                }).then(function (result) {
                  setPlacemark (result.geoObjects.position); 
                  $map_div.show();
                });
              }
              else {
                setPlacemark ([Lat, Lng]); 
                $map_div.show();
              }  
              /**
               * Установка метки (создание или перепозиционирование)
               */
              function setPlacemark(coords) {
                // Если метка уже создана – просто передвигаем ее
                if (myPlacemark) {
                  myPlacemark.geometry.setCoordinates(coords);
                  $lat.val(coords[0]);
                  $lng.val(coords[1]);
                }
                // Если нет – создаем.
                else {
                  myPlacemark = new ymaps.Placemark(coords, { iconContent: ''}, {  preset: 'islands#blueHomeIcon',  draggable: true  });
                  map.geoObjects.add(myPlacemark);
                  // Слушаем событие окончания перетаскивания на метке. 
                  myPlacemark.events.add('dragend', function () {
                     // запишем новые координаты в поля !!
                     var coords = myPlacemark.geometry.getCoordinates();
                     $lat.val(coords[0]);
                     $lng.val(coords[1]);
                  });
                }
                map.setCenter(coords); //map.setCenter(coords, 16); 
              }
            });

					});
				});
			}
		}
	}

})(jQuery);
