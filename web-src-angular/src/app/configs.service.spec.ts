import { TestBed, inject } from '@angular/core/testing';

import { ConfigsService } from './configs.service';

describe('ConfigsService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [ConfigsService]
    });
  });

  it('should be created', inject([ConfigsService], (service: ConfigsService) => {
    expect(service).toBeTruthy();
  }));
});
