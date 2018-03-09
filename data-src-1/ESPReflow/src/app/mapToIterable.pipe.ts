import {Pipe, PipeTransform} from '@angular/core';

@Pipe({
  name: 'MapToIterable',
	pure: true
})
export class MapToIterable implements PipeTransform {
  transform(dict: Object) {
    var a = [];
    for (var key in dict) {
      if (dict.hasOwnProperty(key)) {
        a.push({key: key, val: dict[key]});
      }
    }
    return a;
  }
}
