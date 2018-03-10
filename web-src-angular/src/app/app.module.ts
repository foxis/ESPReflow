import { BrowserModule } from '@angular/platform-browser';
import {FullscreenOverlayContainer, OverlayContainer} from '@angular/cdk/overlay';
import { NgModule } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { ChartsModule } from 'ng2-charts';
import { HttpClientModule } from '@angular/common/http';
import {RouterModule} from '@angular/router';
import {Routes} from '@angular/router';

import {NoopAnimationsModule} from '@angular/platform-browser/animations';

import {
  MatAutocompleteModule,
  //MatBadgeModule,
  //MatBottomSheetModule,
  MatButtonModule,
  MatButtonToggleModule,
  MatCardModule,
  //MatCheckboxModule,
  MatChipsModule,
  //MatDatepickerModule,
  //MatDialogModule,
  //MatDividerModule,
  MatExpansionModule,
  MatFormFieldModule,
  MatGridListModule,
  MatIconModule,
  MatInputModule,
  MatListModule,
  MatMenuModule,
  //MatNativeDateModule,
  MatPaginatorModule,
  //MatProgressBarModule,
  //MatProgressSpinnerModule,
  //MatRadioModule,
  //MatRippleModule,
  MatSelectModule,
  MatSidenavModule,
  MatSliderModule,
  //MatSlideToggleModule,
  //MatSnackBarModule,
  //MatSortModule,
  //MatStepperModule,
  MatTableModule,
  MatTabsModule,
  MatToolbarModule,
  MatTooltipModule//,
  //MatTreeModule
} from '@angular/material';

import { AppComponent } from './app.component';
import { WebsocketService } from './websocket.service';
import { ConfigsService } from './configs.service';
import { MapToIterable } from './mapToIterable.pipe';
import { NavBarComponent } from './nav-bar/nav-bar.component';
import { SetupFormComponent } from './setup-form/setup-form.component';
import { ProfilesFormComponent } from './profiles-form/profiles-form.component';
import { GraphComponent } from './graph/graph.component';
import { MessagesComponent } from './messages/messages.component';

const routes: Routes = [
	{ path: 'graph', component: GraphComponent },
	{ path: 'profiles', component: ProfilesFormComponent },
  { path: 'setup', component: SetupFormComponent },
	{ path: '', redirectTo: '/graph', pathMatch: 'full' },
	{ path: '**', redirectTo: '/graph' }
//	{ path: 'about', component: AboutComponent },
];

@NgModule({
  declarations: [
    AppComponent,
		MapToIterable,
		SetupFormComponent,
		ProfilesFormComponent,
		NavBarComponent,
		GraphComponent,
		MessagesComponent
  ],
  imports: [
		BrowserModule,
		RouterModule.forRoot(routes),
    FormsModule,
		ChartsModule,
		HttpClientModule,
		NoopAnimationsModule,
		MatAutocompleteModule,
    //MatBadgeModule,
    //MatBottomSheetModule,
    MatButtonModule,
    //MatButtonToggleModule,
    MatCardModule,
    //MatCheckboxModule,
    MatChipsModule,
    MatTableModule,
    //MatDatepickerModule,
    //MatDialogModule,
    //MatDividerModule,
    MatExpansionModule,
    MatFormFieldModule,
    MatGridListModule,
    MatIconModule,
    MatInputModule,
    MatListModule,
    MatMenuModule,
    MatPaginatorModule,
    //MatProgressBarModule,
    //MatProgressSpinnerModule,
    //MatRadioModule,
    //MatRippleModule,
    MatSelectModule,
    MatSidenavModule,
    //MatSlideToggleModule,
    MatSliderModule,
    //MatSnackBarModule,
    //MatSortModule,
    //MatStepperModule,
    MatTabsModule,
    MatToolbarModule,
    MatTooltipModule,
    //MatTreeModule,
    //MatNativeDateModule
  ],
  providers: [
		WebsocketService,
		ConfigsService,
		MapToIterable
	],
  bootstrap: [AppComponent]
})
export class AppModule { }
