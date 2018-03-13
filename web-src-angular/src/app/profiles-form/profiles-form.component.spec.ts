import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ProfilesFormComponent } from './profiles-form.component';

describe('ProfilesFormComponent', () => {
  let component: ProfilesFormComponent;
  let fixture: ComponentFixture<ProfilesFormComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ProfilesFormComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ProfilesFormComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
