import {Injectable} from '@angular/core';
import {BehaviorSubject} from 'rxjs/BehaviorSubject';
import {MatPaginator, MatSort} from '@angular/material';
import {DataSource} from '@angular/cdk/collections';
import {Observable} from 'rxjs/Observable';
import {merge} from 'rxjs/observable/merge';
import {map} from 'rxjs/operators/map';


export class Message {
	constructor(str) {
		this.text = str.replace("INFO:", "").replace("WARNING:", "").replace("ERROR:", "").replace("DEBUG:", "");
		if (str.startsWith("INFO:")) {
			this.badge = "INFO";
			this.color = "primary";
		}
		if (str.startsWith("WARNING:")) {
			this.badge = "WARNING";
			this.color = "warn";
		}
		if (str.startsWith("ERROR:")) {
			this.badge = "ERROR";
			this.color = "accent";
		}
		if (str.startsWith("DEBUG:")) {
			this.badge = "DEBUG";
			this.color = "";
		}
	}

	color = "";
	badge = "";
	text = "";

	test(filter) {
		if (filter == 'ALL') {
			return true;
		} else if (filter == "DEBUG") {
			return this.badge == "DEBUG" || this.badge == "ERROR" || this.badge=="WARNING";
		} else if (filter == "INFO") {
			return this.badge == "INFO" || this.badge == "ERROR" || this.badge=="WARNING";
		}
	}
}

@Injectable()
export class MessageDatabase {
  dataChange: BehaviorSubject<Message[]> = new BehaviorSubject<Message[]>([]);

  get data(): Message[] { return this.dataChange.value; }


  constructor() {
    this.initialize();
  }

  initialize() {
    this.dataChange.next([]);
  }

  addMessage(message) {
    const copiedData = this.data.slice();
    copiedData.push(message);
    this.dataChange.next(copiedData);
  }
}

export class MessageDataSource extends DataSource<any> {
  constructor(private _peopleDatabase: MessageDatabase,
              private _paginator: MatPaginator) {
    super();
  }
	public filter = "ALL";

  connect(): Observable<Message[]> {
    const displayDataChanges = [
      this._paginator.page,
      this._peopleDatabase.dataChange
    ];
    return merge(...displayDataChanges).pipe(map(() => {
      const data = this.getFilteredData();
      // Grab the page's slice of data.
      const startIndex = this._paginator.pageIndex * this._paginator.pageSize;
      return data.splice(startIndex, this._paginator.pageSize);
    }));
  }

  disconnect() {
    // No-op
  }

  /** Returns a sorted copy of the database data. */
  getFilteredData(): Message[] {
		const filter = this.filter;
    return this._peopleDatabase.data.slice().filter(x => x.test(filter)).reverse();
  }
}
