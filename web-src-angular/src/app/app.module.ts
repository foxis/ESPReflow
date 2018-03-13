import { BrowserModule } from '@angular/platform-browser';
import {FullscreenOverlayContainer, OverlayContainer} from '@angular/cdk/overlay';
import { NgModule } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { FlexLayoutModule } from '@angular/flex-layout';
import { HttpClientModule } from '@angular/common/http';
import {RouterModule} from '@angular/router';
import {Routes} from '@angular/router';

import { ChartsModule } from 'ng2-charts';

import {NoopAnimationsModule} from '@angular/platform-browser/animations';

import {
  MatAutocompleteModule,
  //MatBadgeModule,
  MatButtonModule,
  MatButtonToggleModule,
  MatCardModule,
  MatChipsModule,
  //MatDialogModule,
  MatExpansionModule,
  MatFormFieldModule,
  MatGridListModule,
  MatIconModule,
  MatInputModule,
  MatListModule,
  MatMenuModule,
  MatPaginatorModule,
  //MatProgressBarModule,
  MatSelectModule,
  MatSidenavModule,
  MatSliderModule,
  //MatSnackBarModule,
  MatTableModule,
  MatTabsModule,
  MatToolbarModule,
  MatTooltipModule//,
} from '@angular/material';

import { AppComponent } from './app.component';
import { WebsocketService } from './websocket.service';
import { ConfigsService } from './configs.service';
import { NavBarComponent } from './nav-bar/nav-bar.component';
import { SetupFormComponent } from './setup-form/setup-form.component';
import { ProfilesFormComponent } from './profiles-form/profiles-form.component';
import { GraphComponent } from './graph/graph.component';
import { MessagesComponent } from './messages/messages.component';

import 'hammerjs';
import { AboutComponent } from './about/about.component';

const routes: Routes = [
	{ path: 'graph', component: GraphComponent },
	{ path: 'profiles', component: ProfilesFormComponent },
	{ path: 'setup', component: SetupFormComponent },
  { path: 'about', component: AboutComponent },
	{ path: '', redirectTo: '/graph', pathMatch: 'full' },
	{ path: '**', redirectTo: '/graph' }
//	{ path: 'about', component: AboutComponent },
];

@NgModule({
  declarations: [
    AppComponent,
		SetupFormComponent,
		ProfilesFormComponent,
		NavBarComponent,
		GraphComponent,
		MessagesComponent,
		AboutComponent
  ],
  imports: [
		BrowserModule,
    FormsModule,
		ChartsModule,
		HttpClientModule,
		NoopAnimationsModule,
		MatAutocompleteModule,
    //MatBadgeModule,
    MatButtonModule,
    MatCardModule,
    MatChipsModule,
    MatTableModule,
    //MatDialogModule,
    MatExpansionModule,
    MatFormFieldModule,
    MatGridListModule,
    MatIconModule,
    MatInputModule,
    MatListModule,
    MatMenuModule,
    MatPaginatorModule,
    //MatProgressBarModule,
    MatSelectModule,
    MatSidenavModule,
    //MatSlideToggleModule,
    MatSliderModule,
    //MatSnackBarModule,
    MatTabsModule,
    MatToolbarModule,
    MatTooltipModule,
		FlexLayoutModule,
		RouterModule.forRoot(routes, {useHash: true})
  ],
  providers: [
		WebsocketService,
		ConfigsService
	],
  bootstrap: [AppComponent]
})
export class AppModule { }
