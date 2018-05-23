int sock = socket(AF_INET , SOCK_STREAM , 0);
mutex=(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
pthread_mutex_init(mutex,NULL);
char* dag_name;
//strcpy(dag_name,argv[1]);
server_port = atoi(argv[2]);
/*******************/
//create DAG
PUBLICROOT=(node*)malloc(sizeof(node));
node*root=PUBLICROOT;
node*temp_node;
char *filename = malloc(sizeof(argv[1]));
filename = argv[1];
int DAG_exists = checkFileAccess(filename);
 if (!DAG_exists) {
     perror("Input FILE does not exist");
     _exit(1);
 }
 temp_node=createDAG(filename);//create DAG data struct.
 root->next=temp_node;
 //test if GAD create successfully
//test();
/****************************/
// Bind it to a local address.
struct sockaddr_in servAddress;
servAddress.sin_family = AF_INET;
servAddress.sin_port = htons(server_port);
servAddress.sin_addr.s_addr = htonl(INADDR_ANY);
bind(sock, (struct sockaddr *) &servAddress, sizeof(servAddress));
listen(sock,MAX_CONNECTIONS);
fprintf(stdout,"Server listening on port %d\n",server_port);
pthread_t test[100];
int i = 0;
// A server typically runs infinitely, with some boolean flag to terminate.
while (1) {
  // Now accept the incoming connections.
  printf("\nDEBUG:IN while(1)\n");
  struct sockaddr_in clientAddress;
  socklen_t size = sizeof(struct sockaddr_in);
  server_IP = malloc(1024);
  server_IP = inet_ntoa(servAddress.sin_addr);
  // TODO: Accept a connection.
  printf("\nDEBUG:before acpet\n");
  int new_socket = accept(sock, (struct sockaddr *)&clientAddress, &size);
  printf("\nDEBUG:after acpet\n");
//	printf("Socket client is %d\n",new_socket);
  struct client_info* info = (struct client_info*) malloc(sizeof(struct client_info));
  info->socket = new_socket;
  fprintf(stdout,"Connection initiated from client at %s:%d\n",inet_ntoa(servAddress.sin_addr),(int) ntohs(servAddress.sin_port));
  client_port = ntohs(clientAddress.sin_port);
  client_IP = malloc(1024);
  client_IP = inet_ntoa(clientAddress.sin_addr);
  pthread_create(&test[i], NULL, runThread, (void*) info);
  pthread_join(test[i],NULL);
  i++;
  close(new_socket);
  fprintf(stdout,"Closed connection with client at %s:%d\n",client_IP, client_port);
}
// Close the server socket.
close(sock);
fprintf(stderr,"\nAll DONE\n");
